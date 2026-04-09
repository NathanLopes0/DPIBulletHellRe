//
// Created by gensh on 01/04/2026.
//

#pragma once
#include "../IAttackStrategy.h"

/**
 * @class LaserAttack
 * @brief Uma estratégia de ataque que dispara projéteis um seguido do outro em direção ao player
 * Recebe parâmetros via struct AttackParams.
 */
class LaserAttack : public IAttackStrategy {
public:
    LaserAttack(ProjectileFactory* spawner, Actor* owner);
    ~LaserAttack() override = default;

    /**
     *
     * @param params Lê os seguintes dados de AttackParams: numProjectiles, projectileSpeed, firePosition (se o laser nao for sair do boss)
     * @return Retorna o vetor com todos os projéteis que são partes desse ataque.
     */
    std::vector<std::unique_ptr<Projectile>> Execute(const AttackParams &params) override;

private:

    ProjectileFactory* mSpawner;
    Actor* mOwner;
};
