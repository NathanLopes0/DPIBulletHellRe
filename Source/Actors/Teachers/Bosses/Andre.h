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

    // Antes o Andre sobrescrevia ExecuteAttack inteiro so para escolher a cor
    // dos baloes. Agora sobrescreve apenas o gancho, e herda automaticamente
    // qualquer correcao feita no fluxo da base.
    void OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                              const std::string& stateName) override;

};
