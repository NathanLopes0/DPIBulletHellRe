//
// Created by nslop on 25/09/2024.
//

#pragma once

#include <memory>

#include "IBossFactory.h"

class Boss;
class Game;

class SallesFactory : public IBossFactory {

public:
    std::unique_ptr<Boss> Create(Game& gameContext) const override;
};
