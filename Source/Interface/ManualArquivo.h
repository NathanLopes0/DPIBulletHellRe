//
// A ponte entre o manual e o disco.
//

#pragma once

#include "Manual.h"

/// Separado de Manual.cpp pelo mesmo motivo dos outros pares do projeto: aquele e puro e entra no
/// alvo de testes.
namespace Manual {

    /// O manual de Assets/como-jogar.json, lido uma vez.
    const Lido& Carregado();
}
