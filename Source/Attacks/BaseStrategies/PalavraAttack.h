//
// Uma palavra de bits, de lado a lado da tela.
//

#pragma once

#include "../IAttackStrategy.h"

/// Uma fileira de 'numProjectiles' bits, um por coluna, de uma borda da tela a outra, descendo
/// juntos a 'projectileSpeed'. A ideia.
class PalavraAttack : public IAttackStrategy {
public:
    PalavraAttack(ProjectileFactory* spawner, Actor* owner);
    ~PalavraAttack() override = default;

    std::vector<std::unique_ptr<Projectile>> Execute(const AttackParams& params) override;
};
