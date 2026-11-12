//
// A ponte entre as materias lidas de arquivo e o jogo.
//

#pragma once

#include "Materias.h"

namespace Materias {

    /// As materias do curso, lidas de Assets/materias.json na primeira chamada.
    const Lista& Carregadas();
}
