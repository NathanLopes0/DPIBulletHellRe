//
// A tabela do Thiago: como o campo de jogo vira linhas e colunas.
//

#include "Tabela.h"

namespace Tabela {

bool Valida(const Forma& forma) {
    return forma.linhas > 0 && forma.colunas > 0;
}

float Espessura(const float tamanho, const int quantas) {
    return (quantas > 0) ? tamanho / static_cast<float>(quantas) : tamanho;
}

float Inicio(const float origem, const float tamanho, const int quantas, const int indice) {
    return origem + Espessura(tamanho, quantas) * static_cast<float>(indice);
}

float Centro(const float origem, const float tamanho, const int quantas, const int indice) {
    return Inicio(origem, tamanho, quantas, indice) + Espessura(tamanho, quantas) / 2.0f;
}

int FaixaDe(const float origem, const float tamanho, const int quantas, const float posicao) {

    if (quantas <= 0) return 0;

    const float espessura = Espessura(tamanho, quantas);
    if (espessura <= 0.0f) return 0;

    const auto crua = static_cast<int>((posicao - origem) / espessura);

    // Ver o comentario de FaixaDe no cabecalho: fora da tabela pertence a borda
    // mais proxima, nunca a "nenhuma faixa".
    if (crua < 0) return 0;
    if (crua >= quantas) return quantas - 1;
    return crua;
}

}
