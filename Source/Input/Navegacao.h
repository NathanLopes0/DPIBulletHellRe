//
// Para onde a selecao vai quando alguem aperta uma seta.
//

#pragma once

#include <cstddef>
#include <vector>

/**
 * CAMADA PURA. So a regra de para onde a selecao anda; nao conhece botao, tecla,
 * materia nem SDL.
 *
 * POR QUE ISTO EXISTE. A forma da grade estava escrita a mao dentro da
 * StageSelect - "a coluna 1 tem 4 itens, a 2 comeca no indice 5" - enquanto os
 * botoes ja eram criados a partir de materias.json. Quando o INF 110 entrou e a
 * grade virou 1,1,4,4,1, os botoes acompanharam e a navegacao nao: a seta para a
 * direita pulava o INF 213, e descer no INF 250 dava voltas num ciclo de quatro
 * que nao existia na tela.
 *
 * Era a QUARTA copia do desenho das materias dentro do C++, depois do mapa de
 * chefes, do desenho da grade e das regras de desbloqueio. As tres primeiras
 * foram embora; esta sobreviveu porque ninguem olha para a navegacao ao mudar
 * uma materia - ate ela quebrar.
 *
 * A grade agora CHEGA PRONTA de quem desenhou os botoes, entao nao ha segunda
 * descricao do layout que possa discordar da primeira.
 */
namespace Navegacao {

    /// colunas[c][linha] = o indice do botao naquela posicao. Montada por quem
    /// cria os botoes, no mesmo laco que os posiciona.
    using Grade = std::vector<std::vector<size_t>>;

    /**
     * @brief O vizinho de cima, dando a volta dentro da propria coluna.
     *
     * Coluna de um item so nao se move: nao ha para onde subir, e devolver outra
     * coisa faria um botao solitario piscar para si mesmo.
     */
    size_t Cima(const Grade& grade, size_t atual);

    /// @brief O vizinho de baixo, dando a volta dentro da propria coluna.
    size_t Baixo(const Grade& grade, size_t atual);

    /**
     * @brief O vizinho da esquerda, dando a volta para a ultima coluna.
     *
     * A linha e preservada quando da, e limitada ao tamanho da coluna de destino
     * quando nao da. Saindo de uma coluna de um item so - onde "a linha atual"
     * nao quer dizer nada - a selecao cai no meio da coluna de destino, que e
     * onde o olho ja estava.
     */
    size_t Esquerda(const Grade& grade, size_t atual);

    /// @brief O vizinho da direita, dando a volta para a primeira coluna.
    size_t Direita(const Grade& grade, size_t atual);

    /// @brief Em que coluna e linha esta um indice. Devolve false se nao estiver
    /// na grade - o que nao deveria acontecer, e por isso e dito em vez de
    /// adivinhado.
    bool Onde(const Grade& grade, size_t atual, size_t& coluna, size_t& linha);
}
