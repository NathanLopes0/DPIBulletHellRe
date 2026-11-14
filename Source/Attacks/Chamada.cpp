//
// O sorteio do Daniel: no comeco da aula, um aluno explica a aula passada.
//
// As caras das cartas (kCartas) estao tambem em Ferramentas/gerar_daniel.py, que as desenha.
//

#include "Chamada.h"

#include <algorithm>
#include <cmath>

namespace Chamada {

    namespace {
        Vector2 Dentro(const Vector2 p, const float margem = 0.06f) {
            return Vector2(std::clamp(p.x, margem, 1.0f - margem), std::clamp(p.y, margem, 1.0f - margem));
        }

        /// Um ponto de uma mancha: em volta de 'centro', mais perto do meio que da beirada.
        Vector2 NaMancha(Variacao::Sorteio& s, const Vector2 centro, const float espalhamento) {
            const float x = (s.Real(-1.0f, 1.0f) + s.Real(-1.0f, 1.0f)) * espalhamento;
            const float y = (s.Real(-1.0f, 1.0f) + s.Real(-1.0f, 1.0f)) * espalhamento;
            return Dentro(centro + Vector2(x, y));
        }

        Vector2 Rumo(const float a) {
            return Vector2(Math::Cos(a), Math::Sin(a));
        }

        /// Um ponto da mancha longe, na horizontal, de 'outro': as duas cartas nao se encostam.
        Vector2 Ao(Variacao::Sorteio& s, const Vector2 centro, const float espalhamento, const Vector2 outro) {
            Vector2 p = NaMancha(s, centro, espalhamento);
            for (int tentativa = 0; tentativa < 40 && std::fabs(p.x - outro.x) < kSeparacao; ++tentativa) {
                p = NaMancha(s, centro, espalhamento * 1.3f);
            }
            if (std::fabs(p.x - outro.x) < kSeparacao) p.x = std::clamp(outro.x + (outro.x < 0.5f ? kSeparacao : -kSeparacao), 0.06f, 0.94f);
            return p;
        }

        /// Poe as tres cartas A, B e C em tres lugares, na ordem sorteada; 'certo' e o indice do
        /// lugar da resposta.
        void Cartas(Questao& q, Variacao::Sorteio& s, const std::array<Vector2, kOpcoes>& lugares, const int certo) {
            std::vector<int> ordem = {0, 1, 2};
            s.Embaralhar(ordem);
            for (int k = 0; k < kOpcoes; ++k) {
                q.cartas[static_cast<size_t>(k)] = lugares[static_cast<size_t>(ordem[static_cast<size_t>(k)])];
                q.caras[static_cast<size_t>(k)] = kCartas[static_cast<size_t>(k)];
                if (ordem[static_cast<size_t>(k)] == certo) q.certa = k;
            }
        }

        /// 'n' centros espalhados pelo quadro, a pelo menos 'distancia' um do outro.
        std::vector<Vector2> Centros(Variacao::Sorteio& s, const int n, const float distancia) {
            std::vector<Vector2> cs;
            for (int tentativa = 0; tentativa < 400 && static_cast<int>(cs.size()) < n; ++tentativa) {
                const Vector2 c(s.Real(0.18f, 0.82f), s.Real(0.2f, 0.8f));
                bool longe = true;
                for (const Vector2& o : cs) longe = longe && (o - c).Length() >= distancia;
                if (longe) cs.push_back(c);
            }
            while (static_cast<int>(cs.size()) < n) cs.emplace_back(0.2f + 0.3f * static_cast<float>(cs.size()), 0.5f);
            return cs;
        }

        Questao Outlier(Variacao::Sorteio& s) {
            Questao q;
            q.tipo = Desafio::Outlier;
            q.enunciado = "Qual dado n\xc3\xa3o combina com os outros?";
            q.resposta = "O que est\xc3\xa1 longe de todos: o outlier";
            const int cor = s.Inteiro(0, 2);
            const Vector2 centro(s.Real(0.38f, 0.62f), s.Real(0.4f, 0.6f));
            for (int k = 0; k < 10; ++k) q.pontos.push_back({NaMancha(s, centro, 0.09f), cor});
            std::array<Vector2, kOpcoes> lugares;
            lugares[0] = Dentro(centro + Rumo(s.Real(0.0f, Math::TwoPi)) * 0.42f, 0.08f);
            lugares[1] = NaMancha(s, centro, 0.07f);
            lugares[2] = Ao(s, centro, 0.07f, lugares[1]);
            for (const Vector2& l : lugares) q.pontos.push_back({l, cor});
            Cartas(q, s, lugares, 0);
            return q;
        }

        Questao CorErrada(Variacao::Sorteio& s) {
            Questao q;
            q.tipo = Desafio::CorErrada;
            q.enunciado = "Qual dado est\xc3\xa1 com a cor errada?";
            q.resposta = "O que tem a cor do outro grupo: o r\xc3\xb3tulo trocado";
            const int a = s.Inteiro(0, 2);
            const int b = (a + s.Inteiro(1, 2)) % 3;
            const Vector2 ca(0.28f, s.Real(0.4f, 0.6f)), cb(0.72f, s.Real(0.4f, 0.6f));
            for (int k = 0; k < 8; ++k) {
                q.pontos.push_back({NaMancha(s, ca, 0.09f), a});
                q.pontos.push_back({NaMancha(s, cb, 0.09f), b});
            }
            std::array<Vector2, kOpcoes> lugares{};
            lugares[0] = NaMancha(s, ca, 0.07f);
            lugares[1] = Ao(s, ca, 0.07f, lugares[0]);
            lugares[2] = NaMancha(s, cb, 0.07f);
            q.pontos.push_back({lugares[0], b});   // a cor do outro grupo, no meio deste
            q.pontos.push_back({lugares[1], a});
            q.pontos.push_back({lugares[2], b});
            Cartas(q, s, lugares, 0);
            return q;
        }

        Questao Grupos(Variacao::Sorteio& s) {
            Questao q;
            q.tipo = Desafio::Grupos;
            q.enunciado = "Quantos grupos tem aqui?";
            const int k = s.Inteiro(2, 4);
            for (const Vector2& c : Centros(s, k, 0.36f)) {
                for (int i = 0; i < 7; ++i) q.pontos.push_back({NaMancha(s, c, 0.065f), -1});
            }
            // As cartas da contagem, embaixo do quadro, em ordem.
            for (int i = 0; i < kOpcoes; ++i) {
                q.cartas[static_cast<size_t>(i)] = Vector2(0.2f + 0.3f * static_cast<float>(i), 1.12f);
                q.caras[static_cast<size_t>(i)] = kCartas[static_cast<size_t>(3 + i)];
            }
            q.certa = k - 2;
            q.resposta = std::to_string(k) + " grupos: " + std::to_string(k) + " manchas separadas";
            return q;
        }

        Questao Centroide(Variacao::Sorteio& s) {
            Questao q;
            q.tipo = Desafio::Centroide;
            q.enunciado = "Qual losango \xc3\xa9 o centr\xc3\xb3ide do grupo?";
            q.resposta = "O centr\xc3\xb3ide \xc3\xa9 a m\xc3\xa9" "dia dos pontos do grupo";
            const int cor = s.Inteiro(0, 2);
            const Vector2 centro(s.Real(0.4f, 0.6f), s.Real(0.42f, 0.58f));
            Vector2 soma;
            for (int k = 0; k < 12; ++k) {
                const Vector2 p = NaMancha(s, centro, 0.12f);
                q.pontos.push_back({p, cor});
                soma += p;
            }
            const Vector2 media = soma * (1.0f / 12.0f);
            const float a = s.Real(0.0f, Math::TwoPi);
            std::array<Vector2, kOpcoes> lugares = {media, Dentro(media + Rumo(a) * 0.3f, 0.08f),
                                                    Dentro(media + Rumo(a + s.Real(2.0f, 4.2f)) * 0.28f, 0.08f)};
            q.losangos.assign(lugares.begin(), lugares.end());
            Cartas(q, s, lugares, 0);
            return q;
        }

        Questao Reta(Variacao::Sorteio& s) {
            Questao q;
            q.tipo = Desafio::Reta;
            q.enunciado = "Qual reta separa as duas cores?";
            q.resposta = "A que deixa cada cor de um lado";
            const float a = s.Real(0.0f, Math::TwoPi);
            const Vector2 meio(0.5f, 0.5f), d = Rumo(a) * 0.2f, lado(-d.y * 2.2f, d.x * 2.2f);
            const Vector2 c0 = meio - d, c1 = meio + d;
            for (int k = 0; k < 8; ++k) {
                q.pontos.push_back({NaMancha(s, c0, 0.07f), 0});
                q.pontos.push_back({NaMancha(s, c1, 0.07f), 1});
            }
            // A que separa (perpendicular, no meio), a que passa pelos dois grupos, e a que corta um deles.
            const std::array<std::array<Vector2, 2>, kOpcoes> retas = {{{meio - lado, meio + lado},
                                                                        {meio - d * 2.2f, meio + d * 2.2f},
                                                                        {c0 - lado, c0 + lado}}};
            std::array<Vector2, kOpcoes> lugares{};
            for (int k = 0; k < kOpcoes; ++k) {
                q.retas.push_back({Dentro(retas[static_cast<size_t>(k)][0], 0.02f), Dentro(retas[static_cast<size_t>(k)][1], 0.02f)});
                lugares[static_cast<size_t>(k)] = q.retas.back()[1];
            }
            Cartas(q, s, lugares, 0);
            return q;
        }

        Questao Vizinho(Variacao::Sorteio& s) {
            Questao q;
            q.tipo = Desafio::Vizinho;
            q.enunciado = "De que grupo \xc3\xa9 o ponto cinza?";
            q.resposta = "Do grupo mais perto: o vizinho decide";
            const std::vector<Vector2> cs = Centros(s, 3, 0.38f);
            for (int g = 0; g < 3; ++g) {
                for (int k = 0; k < 6; ++k) q.pontos.push_back({NaMancha(s, cs[static_cast<size_t>(g)], 0.06f), g});
            }
            const int certo = s.Inteiro(0, 2);
            q.pontos.push_back({Dentro(cs[static_cast<size_t>(certo)] + Rumo(s.Real(0.0f, Math::TwoPi)) * 0.12f), -1});
            std::array<Vector2, kOpcoes> lugares{};
            // Cada carta fica do lado de fora do grupo dela, longe dos outros dois: a letra, e nao a
            // ordem, diz qual e.
            const Vector2 meio = (cs[0] + cs[1] + cs[2]) * (1.0f / 3.0f);
            for (int g = 0; g < 3; ++g) {
                Vector2 fora = cs[static_cast<size_t>(g)] - meio;
                fora = fora.LengthSq() > 1e-6f ? fora * (1.0f / fora.Length()) : Vector2(0.0f, -1.0f);
                lugares[static_cast<size_t>(g)] = Dentro(cs[static_cast<size_t>(g)] + fora * 0.17f, 0.05f);
            }
            std::vector<int> ordem = {0, 1, 2};
            s.Embaralhar(ordem);
            for (int k = 0; k < kOpcoes; ++k) {
                q.cartas[static_cast<size_t>(k)] = lugares[static_cast<size_t>(ordem[static_cast<size_t>(k)])];
                q.caras[static_cast<size_t>(k)] = kCartas[static_cast<size_t>(k)];
                if (ordem[static_cast<size_t>(k)] == certo) q.certa = k;
            }
            return q;
        }
    }

    Questao Gerar(const int aula, Variacao::Sorteio& s) {
        const bool primeiro = s.Chance(0.5f);
        switch (std::clamp(aula, 0, kAulas - 1)) {
            case 0: return primeiro ? Outlier(s) : CorErrada(s);
            case 1: return primeiro ? Grupos(s) : Centroide(s);
            default: return primeiro ? Reta(s) : Vizinho(s);
        }
    }

    bool Alfinete(const Desafio tipo) {
        return tipo == Desafio::Outlier || tipo == Desafio::CorErrada || tipo == Desafio::Centroide;
    }

    int Sortear(Variacao::Sorteio& s, const int alunos) {
        if (alunos <= 1) return s.Inteiro(0, 1) == 0 ? 0 : -1;
        const int r = s.Inteiro(0, 2);
        return r == 2 ? -1 : r;
    }

    std::string Colega(const int k) {
        static constexpr std::array<const char*, kColegas> kNomes = {"Aluno 7", "Aluna 12", "Aluno 23",
                                                                     "Aluna 31", "Aluno 42", "Aluna 5"};
        return kNomes[static_cast<size_t>(((k % kColegas) + kColegas) % kColegas)];
    }

    int NomeNaRoleta(const float t, const int nomes) {
        if (nomes <= 0) return 0;
        // A roleta desacelera: o nome troca a cada 0,06 s no comeco e a cada 0,3 s no fim.
        const float u = std::clamp(t / kRoleta, 0.0f, 1.0f);
        const float trocas = 6.0f * u + 12.0f * (1.0f - (1.0f - u) * (1.0f - u));
        return static_cast<int>(std::floor(trocas)) % nomes;
    }
}
