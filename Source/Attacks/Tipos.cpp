//
// O tipo do aluno no Daniel: fogo, agua ou planta, escolhido no comeco e para a luta inteira.
//

#include "Tipos.h"

#include <cmath>

namespace Tipos {

    namespace {
        bool Valido(const int t) { return t >= 0 && t < kTipos; }
    }

    bool Vence(const int a, const int b) {
        // Fogo (0) vence planta (2), agua (1) vence fogo (0), planta (2) vence agua (1).
        return Valido(a) && Valido(b) && b == (a + 2) % kTipos;
    }

    int QueVence(const int t) {
        return Valido(t) ? (t + 1) % kTipos : -1;
    }

    int Vencido(const int t) {
        return Valido(t) ? (t + 2) % kTipos : -1;
    }

    Efeito Contra(const int doAluno, const int doTiro) {
        if (!Valido(doAluno) || !Valido(doTiro)) return Efeito::Machuca;
        if (doAluno == doTiro) return Efeito::Passa;
        return Vence(doAluno, doTiro) ? Efeito::Absorve : Efeito::Machuca;
    }

    Golpe GolpeContra(const int doTiro, const int doAlvo) {
        if (Vence(doTiro, doAlvo)) return Golpe::Vantagem;
        if (Vence(doAlvo, doTiro)) return Golpe::Resistencia;
        return Golpe::Normal;
    }

    float FatorDoDano(const int doTiro, const int doAlvo) {
        switch (GolpeContra(doTiro, doAlvo)) {
            case Golpe::Vantagem: return kVantagem;
            case Golpe::Resistencia: return kDesvantagem;
            case Golpe::Normal: return 1.0f;
        }
        return 1.0f;
    }

    const char* Nome(const int t) {
        switch (t) {
            case kFogo: return "fogo";
            case kAgua: return "água";
            case kPlanta: return "planta";
            default: return "";
        }
    }

    std::vector<Vector2> LugaresDaEscolha(const float x, const float y, const float largura, const float altura) {
        return {Vector2(x + largura * 0.16f, y + altura * 0.40f), Vector2(x + largura * 0.50f, y + altura * 0.30f),
                Vector2(x + largura * 0.84f, y + altura * 0.40f)};
    }

    int MaisPerto(const std::vector<Vector2>& lugares, const Vector2 p) {
        int melhor = -1;
        float perto = 0.0f;
        for (size_t k = 0; k < lugares.size(); ++k) {
            const float d = (lugares[k] - p).LengthSq();
            if (melhor < 0 || d < perto) {
                melhor = static_cast<int>(k);
                perto = d;
            }
        }
        return melhor;
    }

    std::vector<Particula> Explodir(const int tipo, const Vector2 onde, const int quantas, const float forca,
                                    Variacao::Sorteio& s) {
        std::vector<Particula> v;
        for (int k = 0; k < quantas; ++k) {
            Particula p;
            p.tipo = Valido(tipo) ? tipo : kFogo;
            p.onde = onde;
            const float a = s.Real(0.0f, Math::TwoPi);
            const float r = forca * s.Real(0.35f, 1.0f);
            p.velocidade = Vector2(std::cos(a), std::sin(a)) * r;
            // A fagulha sai mais para cima, a gota espirra para os lados e para cima, a folha solta.
            if (p.tipo == kFogo) p.velocidade.y -= forca * 0.4f;
            if (p.tipo == kAgua) p.velocidade.y -= forca * 0.6f;
            p.vida = s.Real(0.45f, 0.85f) * (p.tipo == kPlanta ? 1.5f : 1.0f);
            p.giro = s.Real(0.0f, Math::TwoPi);
            p.tamanho = s.Real(2.0f, 4.0f);
            v.push_back(p);
        }
        return v;
    }

    void Andar(Particula& p, const float dt) {
        p.idade += dt;
        switch (p.tipo) {
            case kFogo:
                p.velocidade.y -= 90.0f * dt;          // o calor sobe
                p.velocidade = p.velocidade * std::max(0.0f, 1.0f - 2.2f * dt);
                break;
            case kAgua:
                p.velocidade.y += 420.0f * dt;         // a gota cai
                break;
            default:
                p.velocidade.y += 50.0f * dt;          // a folha desce devagar, balancando
                p.velocidade.x += std::sin(p.idade * 9.0f + p.giro) * 120.0f * dt;
                p.velocidade = p.velocidade * std::max(0.0f, 1.0f - 1.6f * dt);
                p.giro += 7.0f * dt;
                break;
        }
        p.onde = p.onde + p.velocidade * dt;
    }
}
