//
// Created by nslop on 26/11/2024.
//

#pragma once

#include "../IAttackStrategy.h"
#include "../../Actors/ProjectileFactory.h"
class Actor;

/**
 * @class CircleSpreadAttack
 * @brief Uma estratégia de ataque que dispara projéteis em um círculo perfeito.
 * Ela recebe os parâmetros (ex: nº de projéteis) via struct AttackParams
 * toda vez que é executada.
 */
class CircleSpreadAttack : public IAttackStrategy {
public:

    CircleSpreadAttack(ProjectileFactory* spawner, Actor* owner);
    ~CircleSpreadAttack() override = default;

    /**
     * @brief Executa o ataque de círculo.
     * @param params O pacote de dados com a configuração (lê numProjectiles,
     * projectileSpeed, firePosition).
     * @return Um vetor de projéteis recém-criados.
     */
    //Assinatura igual da interface IAttackStrategy.
    std::vector<std::unique_ptr<Projectile>> Execute(const AttackParams& params) override;

private:
    // Ponteiros observadores para suas dependências.
    ProjectileFactory* mSpawner;
    Actor* mOwner;
};