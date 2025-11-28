//
// Created by gensh on 27/11/2025.
//

#pragma once
#include "../Boss.h"

class Andre : public Boss {

public:
    Andre(Scene* scene);
    ~Andre() override = default;

    void OnUpdate(float deltaTime) override;

protected:
    void CustomizeAttackParams(AttackParams &params, const std::string &stateName) override;
    void ExecuteAttack(AttackDefinition& attackDef, const std::string& stateName) override;

};
