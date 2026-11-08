//
// O disparo que sai de varios pontos do campo ao mesmo tempo.
//

#pragma once

#include "../IAttackStrategy.h"

/// Um leque de 'numProjectiles' de cada ponto de um posto, e nao do chefe. A ideia.
class PontosAttack : public IAttackStrategy {
public:
    PontosAttack(ProjectileFactory* spawner, Actor* owner);
    ~PontosAttack() override = default;

    std::vector<std::unique_ptr<Projectile>> Execute(const AttackParams& params) override;
};
