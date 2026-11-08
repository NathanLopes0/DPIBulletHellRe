//
// A escada do modelo cascata.
//

#pragma once

#include "../Motor/Math.h"

/// O diagrama do modelo cascata e uma escada: cada etapa e uma caixa um degrau abaixo e a direita
/// da anterior, e a agua so corre para baixo.
namespace Escada {

    /// Onde a escada fica na tela. A largura vem da janela; o resto e o desenho.
    struct Geometria {
        float largura = 1200.0f;
        float margem = 170.0f;    ///< do primeiro e do ultimo degrau ate a borda
        float topo = 110.0f;      ///< a altura do primeiro degrau
        float passo = 55.0f;      ///< quanto cada degrau desce
    };

    /// O centro do degrau k de n, descendo da esquerda para a direita. Fora de [0, n-1], o degrau
    /// mais perto.
    Vector2 Degrau(int k, int n, const Geometria& g);

    /// Em qual degrau se esta no instante 't': um a cada 'segundosPorDegrau', e no ultimo depois do
    /// fim.
    int DegrauNoTempo(float t, float segundosPorDegrau, int n);
}
