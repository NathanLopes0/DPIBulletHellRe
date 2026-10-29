//
// A ponte entre o catalogo de personagens e o disco.
//

#pragma once

#include "Personagens.h"

/**
 * Separado de Personagens.cpp pelo mesmo motivo dos outros pares do projeto:
 * aquele e puro e entra no alvo de testes; este abre arquivo e escreve log,
 * entao fica de fora.
 */
namespace Personagens {

    /**
     * @brief O catalogo de Assets/personagens.json, lido UMA vez.
     *
     * Devolve um catalogo vazio quando o arquivo falta, com o motivo no log.
     * Quem chama deve aguentar isso - um arquivo de arte faltando nao pode
     * impedir alguem de jogar.
     */
    const Catalogo& Carregado();
}
