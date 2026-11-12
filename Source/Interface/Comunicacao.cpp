//
// A comunicacao minima da dupla: o sinal do Botao 3 e o "valeu" de quem foi coberto.
//

#include "Comunicacao.h"

#include <algorithm>
#include <cmath>

namespace Comunicacao {

    bool PodeSinalizar(const float agora, const float ultimo) {
        return ultimo <= kNuncaSinalizou || agora - ultimo >= kEsperaDoSinal;
    }

    float RaioDoSinal(const float t) {
        // De 18 a 72 px, desacelerando: um pulso, e nao uma onda que atravessa o campo.
        const float u = std::clamp(t / kDuracaoDoSinal, 0.0f, 1.0f);
        return 18.0f + 54.0f * (1.0f - (1.0f - u) * (1.0f - u));
    }

    float AlfaDoSinal(const float t) {
        if (t < 0.0f || t >= kDuracaoDoSinal) return 0.0f;
        return 1.0f - t / kDuracaoDoSinal;
    }

    float SubidaDoValeu(const float t) {
        const float u = std::clamp(t / kDuracaoDoValeu, 0.0f, 1.0f);
        return 18.0f * u;
    }

    float AlfaDoValeu(const float t) {
        if (t < 0.0f || t >= kDuracaoDoValeu) return 0.0f;
        // Inteiro ate 60% da vida; depois apaga.
        const float apagando = (t / kDuracaoDoValeu - 0.6f) / 0.4f;
        return apagando <= 0.0f ? 1.0f : 1.0f - apagando;
    }

    bool Juntos(const float umAluno, const float oOutro) {
        if (umAluno <= kNuncaApertou || oOutro <= kNuncaApertou) return false;
        return std::abs(umAluno - oOutro) <= kJanelaDoToque;
    }

    float AlturaDoPulo(const float t) {
        if (t < 0.0f || t >= kDuracaoDoPulo) return 0.0f;
        // Uma parabola: sobe e desce, 16 px no alto.
        const float u = t / kDuracaoDoPulo;
        return 64.0f * u * (1.0f - u);
    }

    float AberturaDoEstouro(const float t) {
        const float u = std::clamp(t / kDuracaoDoEstouro, 0.0f, 1.0f);
        return 1.0f - (1.0f - u) * (1.0f - u);
    }

    float AlfaDoEstouro(const float t) {
        if (t < 0.0f || t >= kDuracaoDoEstouro) return 0.0f;
        return 1.0f - t / kDuracaoDoEstouro;
    }

    const std::vector<Faixa>& Coracao() {
        //  ..XX.XX..
        //  .XXXXXXX.
        //  XXXXXXXXX
        //  XXXXXXXXX
        //  .XXXXXXX.
        //  ..XXXXX..
        //  ...XXX...
        //  ....X....
        static const std::vector<Faixa> kCoracao = {
            {0, 2, 3}, {0, 5, 6}, {1, 1, 7}, {2, 0, 8}, {3, 0, 8},
            {4, 1, 7}, {5, 2, 6}, {6, 3, 5}, {7, 4, 4},
        };
        return kCoracao;
    }
}
