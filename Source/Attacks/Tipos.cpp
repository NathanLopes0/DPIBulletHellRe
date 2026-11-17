//
// O tipo do aluno no Daniel: fogo, agua ou planta, escolhido no comeco e para a luta inteira.
//

#include "Tipos.h"

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

    const char* Nome(const int t) {
        switch (t) {
            case kFogo: return "fogo";
            case kAgua: return "\xc3\xa1gua";
            case kPlanta: return "planta";
            default: return "";
        }
    }

    std::vector<Vector2> LugaresDaEscolha(const float x, const float y, const float largura, const float altura) {
        return {Vector2(x + largura * 0.16f, y + altura * 0.40f), Vector2(x + largura * 0.50f, y + altura * 0.30f),
                Vector2(x + largura * 0.84f, y + altura * 0.40f)};
    }

    float AnguloDoBraco(const int k, const float t, const float sentido) {
        return sentido * kGiroDaRoleta * t + 120.0f * static_cast<float>(k);
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
}
