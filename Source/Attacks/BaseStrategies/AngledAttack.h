//
// Created by nslop on 12/12/2024.
//

#pragma once

#include "../IAttackStrategy.h"
#include <memory>
#include <vector>

class ProjectileFactory;
class Actor;
class Projectile;
struct AttackParams;

/// Uma estratégia que dispara projéteis em um "leque" (ângulo).
class AngledAttack : public IAttackStrategy {
public:
    /// Construtor da estratégia.

    AngledAttack(ProjectileFactory* spawner, Actor* owner);

    ~AngledAttack() override = default;

    /// Executa o ataque em leque.
    std::vector<std::unique_ptr<Projectile>> Execute(const AttackParams& params) override;

private:

    // Função auxiliar pra Execute.
    std::unique_ptr<Projectile> CreateProjectileAtAngle(const AttackParams& params, float AngleInDegrees);
};