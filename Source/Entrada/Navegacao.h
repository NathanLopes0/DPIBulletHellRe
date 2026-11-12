//
// Para onde a selecao vai quando alguem aperta uma seta.
//

#pragma once

#include <cstddef>
#include <vector>

/// So a regra de para onde a selecao anda; nao conhece botao, tecla, materia nem SDL.
namespace Navegacao {

    /// Colunas[c][linha] = o indice do botao naquela posicao. Montada por quem cria os botoes, no
    /// mesmo laco que os posiciona.
    using Grade = std::vector<std::vector<size_t>>;

    /// O vizinho de cima, dando a volta dentro da propria coluna.
    size_t Cima(const Grade& grade, size_t atual);

    /// O vizinho de baixo, dando a volta dentro da propria coluna.
    size_t Baixo(const Grade& grade, size_t atual);

    /// O vizinho da esquerda, dando a volta para a ultima coluna. A linha e preservada quando da, e
    /// limitada ao tamanho da coluna de destino quando nao da.
    size_t Esquerda(const Grade& grade, size_t atual);

    /// O vizinho da direita, dando a volta para a primeira coluna.
    size_t Direita(const Grade& grade, size_t atual);

    /// Em que coluna e linha esta um indice. Devolve false se nao estiver na grade - o que nao
    /// deveria acontecer, e por isso e dito em vez de adivinhado.
    bool Onde(const Grade& grade, size_t atual, size_t& coluna, size_t& linha);
}
