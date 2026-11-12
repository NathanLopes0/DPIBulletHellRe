//
// A ponte entre a exportacao e o disco.
//

#pragma once

#include <string>

#include "../Curso/Materias.h"

namespace Exportacao {

    /// Regrava <base>/Saves/notas.csv com as notas de todos os alunos.
    bool Regravar(const Materias::Lista& materias);
}
