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

    /**
     * @brief As fabricas dos chefes, pelo NOME - o mesmo do campo "chefe" de
     * materias.json e da pasta em Assets/Attacks/.
     *
     * POR NOME, e nao por materia: quem diz qual chefe cada materia tem e o
     * arquivo (Game::GetFactory). Um chefe novo e uma linha em Chefes.cpp.
     */
    std::map<std::string, std::unique_ptr<IBossFactory>> Fabricas(Game* game);
}
