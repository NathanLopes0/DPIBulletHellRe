//
// Created by gensh on 26/11/2025.
//

#include "Ricardo.h"

#include <SDL_log.h>

Ricardo::Ricardo(Scene *scene) : Boss(scene)
{
}

void Ricardo::OnUpdate(const float deltaTime) {
    Boss::OnUpdate(deltaTime);
}

void Ricardo::CustomizeAttackParams(AttackParams &params, const std::string &stateName) {
    Boss::CustomizeAttackParams(params, stateName);
}

