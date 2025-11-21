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

/**
 * @class WaveAttack
 * @brief Uma estratégia que dispara projéteis em um "leque", um após o outro,
 * criando um efeito de "onda".
 * Esta classe é stateless e recebe sua configuração (como creationSpeed)
 * via struct AttackParams.
 */
class WaveAttack : public IAttackStrategy {
public:

    WaveAttack(ProjectileFactory* spawner, Actor* owner);
    ~WaveAttack() override = default;

    /**
     * @brief Executa o ataque em onda.
     * @param params O pacote de dados com a configuração (lê numProjectiles,
     * angle, creationSpeed, etc.).
     * @return Um vetor de projéteis recém-criados.
     */
    std::vector<std::unique_ptr<Projectile>> Execute(const AttackParams& params) override;

private:
    ProjectileFactory* mSpawner;
    Actor* mOwner;
};