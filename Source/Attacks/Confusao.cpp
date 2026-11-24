//
// A final do Daniel: a matriz de confusao.
//

#include "Confusao.h"

#include <algorithm>

namespace Confusao {

    Vector2 CentroDaCelula(const Grade& g, const int real, const int previsto) {
        const float lw = g.w / static_cast<float>(kTipos);
        const float lh = g.h / static_cast<float>(kTipos);
        return Vector2(g.x + lw * (static_cast<float>(std::clamp(previsto, 0, kTipos - 1)) + 0.5f),
                       g.y + lh * (static_cast<float>(std::clamp(real, 0, kTipos - 1)) + 0.5f));
    }

    Vector2 LugarNaCelula(const Grade& g, const int real, const int previsto, const int k) {
        const float lw = g.w / static_cast<float>(kTipos);
        // Lado a lado, no meio da casa, com uma folga nas bordas.
        const float passo = lw * 0.8f / static_cast<float>(kPorCelula);
        const int i = ((k % kPorCelula) + kPorCelula) % kPorCelula;
        const float dx = (static_cast<float>(i) - (kPorCelula - 1) * 0.5f) * passo;
        return CentroDaCelula(g, real, previsto) + Vector2(dx, 6.0f);
    }

    int Previsto(Variacao::Sorteio& s, const int real, const float acuracia) {
        if (s.Chance(acuracia)) return real;
        // Um dos outros dois, por igual.
        const int desvio = s.Inteiro(1, kTipos - 1);
        return (real + desvio) % kTipos;
    }

    float Acuracia(const Matriz& m) {
        int total = 0, certos = 0;
        for (int r = 0; r < kTipos; ++r) {
            for (int p = 0; p < kTipos; ++p) {
                total += m[static_cast<size_t>(r)][static_cast<size_t>(p)];
                if (r == p) certos += m[static_cast<size_t>(r)][static_cast<size_t>(p)];
            }
        }
        return total > 0 ? static_cast<float>(certos) / static_cast<float>(total) : 0.0f;
    }

    int Porcento(const Matriz& m) {
        return static_cast<int>(Acuracia(m) * 100.0f + 0.5f);
    }

    Degrau DegrauDo(const Dificuldade::Nivel nivel) {
        Degrau g;
        if (nivel == Dificuldade::Nivel::Dificil) {
            g.entreAmostras = 0.7f;
            g.naSegundaEtapa = 0.5f;
            g.acuracia = 0.4f;
            g.velocidade = 340.0f;
            g.errosNoCampo = 6;
            g.vidaDoErro = 3.0f;
            g.paciencia = 3.6f;
            g.entreTirosDoErro = 1.1f;
            g.velocidadeDoErro = 230.0f;
            g.tirosDoVazamento = 16;
            g.velocidadeDoVazamento = 150.0f;
            g.premioDaCorrecao = 0.6f;
            g.entreLeques = 1.6f;
            g.tirosDoLeque = 8;
        }
        return g;
    }
}
