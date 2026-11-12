//
// A hora do sistema, isolada num lugar so.
//

#pragma once

#include <ctime>
#include <string>

/// O relogio e a unica coisa do mundo externo de que a ficha do aluno precisa, e por isso ele mora
/// aqui em vez de dentro dela.
namespace Relogio {

    /// Um instante como "2026-10-02 14:30:00", em hora local.
    std::string Formatar(std::time_t instante);

    std::string Agora();
}
