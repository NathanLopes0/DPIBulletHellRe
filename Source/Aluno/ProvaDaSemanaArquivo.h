//
// A ponte entre a prova da semana e o disco.
//

#pragma once

#include <vector>

#include "ProvaDaSemana.h"

namespace ProvaDaSemana {

    /// Tudo o que ha em <base>/Saves/prova_da_semana.csv. Vazio se o arquivo nao existe.
    std::vector<Entrada> Carregar();

    /// Acrescenta uma nota ao arquivo (com o cabecalho, se ele nasce agora). So acrescenta: nada ja
    /// escrito e reescrito.
    bool Acrescentar(const Entrada& e);
}
