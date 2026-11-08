//
// Os chefes do jogo: o nome de cada um e a fabrica que o monta.
//

#pragma once

#include <map>
#include <memory>
#include <string>

#include "IBossFactory.h"

class Game;

namespace Chefes {

    /// As fabricas dos chefes, pelo nome - o mesmo do campo "chefe" de materias.json e da pasta em
    /// Assets/Attacks/.
    std::map<std::string, std::unique_ptr<IBossFactory>> Fabricas(Game* game);
}
