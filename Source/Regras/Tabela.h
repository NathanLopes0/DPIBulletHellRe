//
// A tabela do Thiago: como o campo de jogo vira linhas e colunas.
//

#pragma once

/// So a aritmetica de dividir um intervalo em faixas iguais; nao conhece projetil, chefe, cena nem
/// SDL.
namespace Tabela {

    /// Quantas linhas e colunas uma tabela tem. Minimos de 1 sao garantidos por
    /// Valida(), porque uma tabela de zero linhas nao tem onde varrer.
    struct Forma {
        int linhas = 4;
        int colunas = 5;
    };

    /// A forma e utilizavel? Zero ou negativo nao e.
    bool Valida(const Forma& forma);

    /// A espessura de uma faixa: o intervalo dividido em partes iguais.
    float Espessura(float tamanho, int quantas);

    /// Onde comeca a faixa de indice `indice`.
    float Inicio(float origem, float tamanho, int quantas, int indice);

    /// O meio da faixa - onde a varredura e centrada.
    float Centro(float origem, float tamanho, int quantas, int indice);

}
