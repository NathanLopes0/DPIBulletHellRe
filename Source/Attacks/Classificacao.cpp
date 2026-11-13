//
// A fase 3 do Daniel: o aprendizado supervisionado e os comites.
//

#include "Classificacao.h"

#include <algorithm>
#include <cmath>

namespace Classificacao {

    int EtapaNoTempo(const float t) {
        float fim = 0.0f;
        for (size_t k = 0; k < kDuracoes.size(); ++k) {
            fim += kDuracoes[k];
            if (t < fim) return static_cast<int>(k);
        }
        return static_cast<int>(kDuracoes.size()) - 1;
    }

    int Lado(const Reta& r, const Vector2 p) {
        return r.a * p.x + r.b * p.y + r.c >= 0.0f ? 1 : 0;
    }

    Reta RetaPeloAngulo(const float graus, const float deslocamento) {
        const float a = Math::ToRadians(graus);
        return Reta{Math::Cos(a), Math::Sin(a), -deslocamento};
    }

    bool Segmento(const Reta& r, Vector2& de, Vector2& ate) {
        // Os pontos em que a reta cruza as quatro bordas da caixa, e os dois mais distantes entre si.
        std::vector<Vector2> cruza;
        const auto acrescentar = [&cruza](const Vector2 p) {
            if (p.x < -1.0001f || p.x > 1.0001f || p.y < -1.0001f || p.y > 1.0001f) return;
            for (const Vector2& q : cruza) if ((q - p).LengthSq() < 1e-8f) return;
            cruza.push_back(p);
        };
        if (std::fabs(r.b) > 1e-6f) {
            for (const float x : {-1.0f, 1.0f}) acrescentar(Vector2(x, -(r.a * x + r.c) / r.b));
        }
        if (std::fabs(r.a) > 1e-6f) {
            for (const float y : {-1.0f, 1.0f}) acrescentar(Vector2(-(r.b * y + r.c) / r.a, y));
        }
        if (cruza.size() < 2) return false;
        float maior = -1.0f;
        for (size_t i = 0; i < cruza.size(); ++i) {
            for (size_t j = i + 1; j < cruza.size(); ++j) {
                const float d = (cruza[i] - cruza[j]).LengthSq();
                if (d > maior) {
                    maior = d;
                    de = cruza[i];
                    ate = cruza[j];
                }
            }
        }
        return true;
    }

    Epoca Treinar(const Reta& modelo, const Reta& verdade, Variacao::Sorteio& s, const int amostras, const float taxa) {
        Epoca e{modelo, 0};
        for (int k = 0; k < amostras; ++k) {
            const Vector2 p(s.Real(-1.0f, 1.0f), s.Real(-1.0f, 1.0f));
            const int y = Lado(verdade, p);
            const int previsto = Lado(e.modelo, p);
            if (y == previsto) continue;
            ++e.erros;
            const float passo = taxa * static_cast<float>(y - previsto);
            e.modelo.a += passo * p.x;
            e.modelo.b += passo * p.y;
            e.modelo.c += passo;
        }
        const float n = std::sqrt(e.modelo.a * e.modelo.a + e.modelo.b * e.modelo.b);
        if (n > 1e-6f) {
            e.modelo.a /= n;
            e.modelo.b /= n;
            e.modelo.c /= n;
        } else {
            e.modelo = modelo;
        }
        return e;
    }

    int Classe(const Arvore& a, const Vector2 p) {
        const int lado = p.x < a.corteX ? 0 : 1;
        const int baixo = p.y < a.corteY[static_cast<size_t>(lado)] ? 0 : 1;
        return a.folhas[static_cast<size_t>(2 * lado + baixo)];
    }

    int Voto(const std::vector<Arvore>& comite, const Vector2 p) {
        int votos = 0;
        for (const Arvore& a : comite) votos += Classe(a, p);
        return 2 * votos > static_cast<int>(comite.size()) ? 1 : 0;
    }

    int Verdade(const Vector2 cortes, const Vector2 p) {
        return (p.x < cortes.x) != (p.y < cortes.y) ? 1 : 0;
    }

    Arvore Plantar(Variacao::Sorteio& s, const Vector2 cortes, const float ruido, const float chanceDeErro) {
        Arvore a;
        a.corteX = std::clamp(cortes.x + s.Real(-ruido, ruido), -0.85f, 0.85f);
        for (float& y : a.corteY) y = std::clamp(cortes.y + s.Real(-ruido, ruido), -0.85f, 0.85f);
        // As folhas sao as da verdade nas quatro quinas; uma ou outra sai trocada.
        for (int lado = 0; lado < 2; ++lado) {
            for (int baixo = 0; baixo < 2; ++baixo) {
                const Vector2 quina(lado == 0 ? -1.0f : 1.0f, baixo == 0 ? -1.0f : 1.0f);
                int f = Verdade(cortes, quina);
                if (s.Real(0.0f, 1.0f) < chanceDeErro) f = 1 - f;
                a.folhas[static_cast<size_t>(2 * lado + baixo)] = f;
            }
        }
        return a;
    }

    int ClasseDaRajada(Variacao::Sorteio& s, const int anterior) {
        if (anterior < 0) return s.Inteiro(0, kClasses - 1);
        return s.Real(0.0f, 1.0f) < 0.78f ? 1 - anterior : anterior;
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n != Dificuldade::Nivel::Dificil) return d;
        d.entreRajadas = 0.52f;
        d.tirosDaRajada = 17;
        d.velocidade = 280.0f;
        d.entreEpocas = 1.6f;
        d.entreReplantios = 1.9f;
        d.aviso = 0.45f;
        d.entreChuvas = 0.065f;
        return d;
    }
}
