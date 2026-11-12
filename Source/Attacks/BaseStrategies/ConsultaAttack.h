//
// A varredura de uma linha ou coluna da tabela.
//

#pragma once

#include "../IAttackStrategy.h"

/// Varre uma faixa da tabela - uma linha ou uma coluna - de ponta a ponta. A ideia.
class ConsultaAttack : public IAttackStrategy {
public:
    ConsultaAttack(ProjectileFactory* spawner, Actor* owner);
    ~ConsultaAttack() override = default;

    std::vector<std::unique_ptr<Projectile>> Execute(const AttackParams& params) override;
};
