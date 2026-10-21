//
// A capsula: in capsula, "na caixinha".
//

#pragma once

#include <vector>

#include "../Math.h"

/**
 * CAMADA PURA.
 *
 * Na fase 2 do Luizinho (Encapsulamento) as letras viajam DENTRO de uma capsa,
 * o estojo em que se guardavam os rolos e de onde vem "capsula", a caixinha. O
 * aluno so ve a casca; o que tem dentro so aparece quando ela abre, e ela so
 * abre por dentro, no tempo dela. Aberta, ela vira um anel de letras.
 *
 * Quem faz isso no jogo e CapsulaAttack; aqui fica a geometria da abertura.
 */
namespace Capsula {

    /**
     * @brief As velocidades das 'n' letras quando a capsula abre: um anel
     * igualmente espacado, com modulo 'velocidade'.
     *
     * O ANEL E MEDIDO A PARTIR DO VOO ('anguloDoVoo', em graus, 0 = direita e
     * 90 = baixo, como o resto do jogo): a primeira letra segue em frente, para
     * onde a capsula ia. Assim uma letra do anel continua sempre em direcao ao
     * lugar em que o aluno estava no disparo, qualquer que seja o 'n'.
     */
    std::vector<Vector2> Abertura(int n, float anguloDoVoo, float velocidade);
}
