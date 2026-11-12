//
// Os campos que so a CaixaAttack usa.
//

#pragma once

#include "AttackParams.h"

/// Uma caixa: a forma do retangulo e o tamanho em que ele nasce.
struct CaixaAttackParams : AttackParams {

    /// Largura dividida pela altura. 1,6 e a proporcao de uma janela deitada,
    /// que e o que uma div parece numa tela.
    float proporcao = 1.6f;

    /// A meia-altura com que a caixa nasce, em pixels.
    float meiaAlturaInicial = 72.0f;
};
