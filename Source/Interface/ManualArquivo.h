//
// A ponte entre o manual e o disco.
//

#pragma once

#include "Manual.h"

/**
 * Separado de Manual.cpp pelo mesmo motivo dos outros pares do projeto:
 * aquele e puro e entra no alvo de testes; este abre arquivo e escreve log,
 * entao fica de fora.
 */
namespace Manual {

    /**
     * @brief O manual de Assets/como-jogar.json, lido UMA vez.
     *
     * Sem o arquivo, o manual vem vazio e o motivo vai para o log; a tela
     * mostra um aviso no lugar das paginas. Um texto faltando nao pode impedir
     * ninguem de jogar.
     */
    const Lido& Carregado();
}
