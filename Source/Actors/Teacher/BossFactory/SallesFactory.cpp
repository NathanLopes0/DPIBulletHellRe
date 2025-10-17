//
// Created by nslop on 25/09/2024.
//

#include <string>
#include "../Boss.h"
#include "../../../Game.h"
#include "SallesFactory.h"
#include "../Bosses/Salles.h"

std::unique_ptr<Boss> SallesFactory::Create(Game& gameContext) const {

    auto salles = std::make_unique<Salles>(&gameContext);

    std::string spritePath = "../Assets/Teachers/DPIBHSalles.png";
    std::string dataPath = "../Assets/Teachers/DPIBHSalles.json";

    return salles;




}
