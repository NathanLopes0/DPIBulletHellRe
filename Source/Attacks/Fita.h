//
// A fita da maquina de Turing do Vladimir - a regra de quando uma casa e
// reescrita, sem nada do motor.
//

#pragma once

#include <vector>

/**
 * CAMADA PURA (mesma disciplina de EscopoQueCresce e PathAim).
 *
 * A fase final do Vladimir e a maquina de Turing: o chefe escreve a fita na
 * passagem - simbolos que descem e param numa fileira - e, quando volta por
 * cima, LE cada casa e a REESCREVE. O "a" parado vira "b" e sai perseguindo. E
 * a transicao que o placar anuncia: delta(q, a) = (q, b, ->).
 *
 * Quem faz isso no jogo e ReescreverBehavior (Behaviors.h), que precisa do
 * projetil, da cena e dos corpos do chefe - e nada disso entra na suite. O que
 * da para separar e a DECISAO, e e nela que mora o erro que importa: a casa
 * nasce exatamente embaixo da cabeca, entao sem a idade minima toda casa seria
 * reescrita no quadro em que foi escrita, e a fita nunca existiria.
 */
namespace Fita {

    /**
     * @brief A cabeca esta em cima da casa?
     *
     * SO O X CONTA. A cabeca le a casa que esta embaixo dela, a qualquer
     * altura: o chefe varre por cima da fita, e nunca na mesma linha que ela.
     * O raio e inclusivo - uma cabeca exatamente na borda le a casa.
     */
    bool CabecaSobreACasa(float xDaCabeca, float xDaCasa, float raio);

    /**
     * @brief Esta casa deve ser reescrita agora?
     *
     * Precisa das duas coisas: a casa ja ter a idade minima - para nao ser
     * reescrita pela cabeca que acabou de escreve-la - e ALGUMA cabeca estar em
     * cima dela. "Alguma" porque o chefe pode estar dividido em dois corpos
     * (Battle::Corpos), e qualquer um deles e uma cabeca.
     *
     * Sem cabeca nenhuma em campo, nada e reescrito.
     */
    bool DeveReescrever(float idade, float idadeMinima,
                        const std::vector<float>& xDasCabecas, float xDaCasa, float raio);
}
