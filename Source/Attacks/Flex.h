//
// O flexbox do CSS - onde cada item de uma fileira fica, para cada valor de
// justify-content, e a curva da transicao entre dois arranjos.
//

#pragma once

#include <string>
#include <vector>

/**
 * CAMADA PURA (mesma disciplina de Caixa, Fita e Tabela).
 *
 * A fase 2 do Lucas e o CSS: uma fileira de regras cai arrumada de um jeito e,
 * no meio da queda, se rearranja de outro - como um justify-content trocado
 * numa folha de estilo. Quem cria os projeteis e FlexAttack; aqui ficam as
 * contas que podem errar.
 */
namespace Flex {

    /// Os cinco valores de justify-content que a fase usa, com os nomes do CSS.
    enum class Justificar { Inicio, Fim, Centro, EspacoEntre, EspacoEmVolta };

    /**
     * @brief Le o valor pelo nome do CSS: flex-start, flex-end, center,
     * space-between ou space-around. Devolve false para qualquer outro nome.
     */
    bool PeloNome(const std::string& nome, Justificar& saida);

    /// @brief O nome do CSS de volta - e o que o placar escreve.
    std::string Nome(Justificar j);

    /**
     * @brief O centro de cada um dos 'n' itens, de largura 'item', numa linha
     * que vai de 'inicio' a 'fim'. Exatamente as regras do CSS:
     *
     *   flex-start      encostados no inicio, sem espaco entre eles
     *   flex-end        encostados no fim
     *   center          encostados uns nos outros, no meio da linha
     *   space-between   o primeiro no inicio, o ultimo no fim, o resto dividido
     *   space-around    o mesmo espaco em volta de cada um (meio espaco nas pontas)
     *
     * Com um item so, space-between se comporta como flex-start, como no CSS.
     * Sem item, nao ha posicao.
     */
    std::vector<float> Posicoes(Justificar j, int n, float item, float inicio, float fim);

    /**
     * @brief Quanto da transicao ja aconteceu, de 0 a 1, no instante 't'.
     *
     * Zero ate 'atraso', um depois de 'atraso + duracao', e no meio a curva
     * suave (ease-in-out) que o CSS usa por padrao numa transition: comeca e
     * termina devagar. Duracao zero ou negativa vira um degrau.
     */
    float Progresso(float t, float atraso, float duracao);
}
