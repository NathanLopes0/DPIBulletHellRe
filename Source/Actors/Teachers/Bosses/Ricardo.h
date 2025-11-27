//
// Created by gensh on 26/11/2025.
//

#pragma once

#include "../Boss.h"

class Ricardo : public Boss {
public:
    Ricardo(Scene* scene);
    ~Ricardo() override = default;
    void OnUpdate(float deltaTime) override;

protected:
    void CustomizeAttackParams(AttackParams &params, const std::string &stateName) override;
};
