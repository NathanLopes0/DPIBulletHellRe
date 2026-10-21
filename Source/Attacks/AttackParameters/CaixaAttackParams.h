//
// Os campos que so a CaixaAttack usa.
//

#pragma once

#include "AttackParams.h"

/**
 * @brief Uma caixa: a forma do retangulo e o tamanho em que ele nasce.
 *
 * Mesmo padrao do BaloonAttackParams e do ConsultaAttackParams: a CaixaAttack
 * faz um dynamic_cast para esta struct. Aqui o bloco "caixa" e OPCIONAL em
 * fases.json - a ponte constroi esta struct para todo ataque CaixaAttack, e um
 * campo ausente fica com o padrao abaixo.
 */
struct CaixaAttackParams : AttackParams {

    /// Largura dividida pela altura. 1,6 e a proporcao de uma janela deitada,
    /// que e o que uma div parece numa tela.
    float proporcao = 1.6f;

    /// A meia-altura com que a caixa NASCE, em pixels. Precisa ser maior que a
    /// metade do cartao do chefe (64): menor, a caixa nasceria por dentro dele
    /// e os primeiros quadros dela seriam escondidos pelo proprio professor.
    float meiaAlturaInicial = 72.0f;
};
