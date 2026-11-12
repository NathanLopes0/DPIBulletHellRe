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

}
