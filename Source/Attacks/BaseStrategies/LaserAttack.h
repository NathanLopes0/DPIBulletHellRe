//
// Created by gensh on 01/04/2026.
//

#pragma once
#include "../IAttackStrategy.h"

/// Uma estratégia de ataque que dispara projéteis um seguido do outro em direção ao player Recebe
/// parâmetros via struct AttackParams.
class LaserAttack : public IAttackStrategy {
public:
    LaserAttack(ProjectileFactory* spawner, Actor* owner);
    ~LaserAttack() override = default;

    std::vector<std::unique_ptr<Projectile>> Execute(const AttackParams &params) override;

private:

};
