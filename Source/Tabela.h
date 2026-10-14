//
// A tabela do Thiago: como o campo de jogo vira linhas e colunas.
//

#pragma once

/**
 * CAMADA PURA. So a aritmetica de dividir um intervalo em faixas iguais; nao
 * conhece projetil, chefe, cena nem SDL.
 *
 * POR QUE ISTO EXISTE SEPARADO. Na batalha do Thiago, DUAS pessoas precisam
 * concordar sobre onde fica a linha 2: a estrategia, que posiciona a varredura,
 * e o chefe, que decide QUAL linha varrer a partir de onde o jogador esta. Se
 * cada uma fizesse a propria conta, bastaria um arredondamento diferente para o
 * jogo telegrafar uma linha e varrer outra - e o jogador seria atingido por um
 * ataque que ele leu certo.
 *
 * Esse erro ja aconteceu neste projeto com a grade da selecao de fases: duas
 * descricoes do mesmo layout, uma delas desatualizada. Aqui ha so uma.
 *
 * CONVENCAO: as faixas sao numeradas de 0 a quantas-1, na ordem crescente do
 * eixo. Para linhas, a faixa 0 e a de cima (y menor); para colunas, a da
 * esquerda (x menor).
 */
namespace Tabela {

    /// Quantas linhas e colunas uma tabela tem. Minimos de 1 sao garantidos por
    /// Valida(), porque uma tabela de zero linhas nao tem onde varrer.
    struct Forma {
        int linhas = 4;
        int colunas = 5;
    };

    /// @brief A forma e utilizavel? Zero ou negativo nao e.
    bool Valida(const Forma& forma);

    /// @brief A espessura de uma faixa: o intervalo dividido em partes iguais.
    float Espessura(float tamanho, int quantas);

    /// @brief Onde comeca a faixa de indice `indice`.
    float Inicio(float origem, float tamanho, int quantas, int indice);

    /// @brief O meio da faixa - onde a varredura e centrada.
    float Centro(float origem, float tamanho, int quantas, int indice);

    /**
     * @brief Em que faixa cai uma posicao.
     *
     * LIMITADO AOS EXTREMOS de proposito. Um jogador exatamente na borda, ou
     * meio pixel fora dela por causa do colisor, tem de continuar pertencendo a
     * alguma linha: devolver -1 ali faria o chefe varrer uma linha inexistente e
     * o ataque sumir, o que o jogador leria como "o chefe travou".
     */
    int FaixaDe(float origem, float tamanho, int quantas, float posicao);
}
