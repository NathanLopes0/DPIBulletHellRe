//
// A ponte entre o catalogo de personagens e o disco.
//

#pragma once

#include "Personagens.h"

/// Separado de Personagens.cpp pelo mesmo motivo dos outros pares do projeto: aquele e puro e entra
/// no alvo de testes.
namespace Personagens {

    /// O catalogo de Assets/personagens.json, lido uma vez. Devolve um catalogo vazio quando o
    /// arquivo falta, com o motivo no log.
    const Catalogo& Carregado();
}
