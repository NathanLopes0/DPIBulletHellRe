//
// Created by gensh on 17/10/2025.
//

#pragma once
#include <memory>
#include <vector>
#include "AttackParameters/AttackParams.h"
#include "../Actors/Actor.h"
#include "../Actors/ProjectileFactory.h"

class Projectile;

/// Define o contrato para qualquer padrão de ataque no jogo.
class IAttackStrategy {
public:

    // Ficam aqui para que Acquire() possa ser implementado uma única vez na base.
    IAttackStrategy(ProjectileFactory* spawner, Actor* owner)
        : mSpawner(spawner), mOwner(owner) {}

    virtual ~IAttackStrategy() = default;

    /// Executa o padrão de ataque.
    virtual std::vector<std::unique_ptr<Projectile>> Execute(const AttackParams& params) = 0;

protected:

    /// Pede um Projétil pronto (novo ou reciclado do pool) para o mSpawner desta strategy, usando o
    /// mOwner já conhecido desde a construção.
    std::unique_ptr<Projectile> Acquire() const {
        return mSpawner->Acquire(mOwner->GetScene(), mOwner);
    }

    ProjectileFactory* mSpawner;
    Actor* mOwner;
};