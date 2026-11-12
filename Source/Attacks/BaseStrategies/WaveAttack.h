//
// Created by nslop on 17/12/2024.
//

#pragma once

#include "../IAttackStrategy.h"
#include <memory>
#include <vector>

class ProjectileFactory;
class Actor;
class Projectile;
class AttackParams;

/// Uma estratégia que dispara projéteis em um "leque", um após o outro, criando um efeito de
/// "onda".
class WaveAttack : public IAttackStrategy {
public:

    WaveAttack(ProjectileFactory* spawner, Actor* owner);
    ~WaveAttack() override = default;

    /// Executa o ataque em onda.
    std::vector<std::unique_ptr<Projectile>> Execute(const AttackParams& params) override;

private:
};