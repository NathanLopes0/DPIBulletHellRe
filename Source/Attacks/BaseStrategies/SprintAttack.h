//
// A sprint dos metodos ageis: o cursor arrasta as historias para o quadro.
//

#pragma once

#include "../IAttackStrategy.h"

/// Uma sprint de 'numProjectiles' historias, arrastadas por um cursor do professor ate o quadro,
/// onde esperam a revisao. A ideia.
class SprintAttack : public IAttackStrategy {
public:
    SprintAttack(ProjectileFactory* spawner, Actor* owner);
    ~SprintAttack() override = default;

    std::vector<std::unique_ptr<Projectile>> Execute(const AttackParams& params) override;
};
