//
// A ponte entre o registro das tentativas e o disco.
//

#pragma once

#include "Tentativas.h"

namespace TentativasArquivo {

    /// Acrescenta a tentativa a <base>/Saves/tentativas.csv e tentativas_fases.csv, e refaz
    /// tentativas_resumo.csv.
    bool Gravar(const Tentativas::Tentativa& tentativa);
}
