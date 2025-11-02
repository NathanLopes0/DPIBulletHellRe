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

/**
 * @class AngledAttack
 * @brief Uma estratégia que dispara projéteis em um "leque" (ângulo).
 * Esta classe é "stateless" e recebe sua configuração (ângulo,
 * velocidade, etc.) via struct AttackParams.
 */
class AngledAttack : public IAttackStrategy {
public:
    /**
     * @brief Construtor da estratégia.
     * @param spawner A fábrica que usará para criar os projéteis.
     * @param owner o Actor que quer criar os projéteis.
     */

    AngledAttack(ProjectileFactory* spawner, Actor* owner);

    ~AngledAttack() override = default;

    /**
     * @brief Executa o ataque em leque.
     * @param params O pacote de dados com a configuração (lê angle,
     * centralAngle, numProjectiles, projectileSpeed, firePosition).
     * @return Um vetor de projéteis recém-criados.
     */
    std::vector<std::unique_ptr<Projectile>> Execute(const AttackParams& params) override;

private:
    // Armazena apenas ponteiros observadores para suas dependências.
    ProjectileFactory* mSpawner;
    Actor* mOwner;


    // Função auxiliar pra Execute
    std::unique_ptr<Projectile> CreateProjectileAtAngle(const AttackParams& params, float AngleInDegrees);
};