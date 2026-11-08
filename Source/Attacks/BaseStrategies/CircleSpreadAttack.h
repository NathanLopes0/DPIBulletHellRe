//
// Created by nslop on 26/11/2024.
//

#pragma once

#include "../IAttackStrategy.h"
#include "../../Actors/ProjectileFactory.h"
class Actor;

/// Uma estratégia de ataque que dispara projéteis em um círculo perfeito.
class CircleSpreadAttack : public IAttackStrategy {
public:

    CircleSpreadAttack(ProjectileFactory* spawner, Actor* owner);
    ~CircleSpreadAttack() override = default;

    /// Executa o ataque de círculo.
    std::vector<std::unique_ptr<Projectile>> Execute(const AttackParams& params) override;

private:
};