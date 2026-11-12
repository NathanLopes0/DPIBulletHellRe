//
// A caixa do HTML: um retangulo de projeteis que nasce em volta do chefe e
// cresce sem perder a forma.
//

#pragma once

#include "../IAttackStrategy.h"

/// Um retangulo de projeteis em volta do chefe, que cresce como uma div. A ideia.
class CaixaAttack : public IAttackStrategy {
public:
    CaixaAttack(ProjectileFactory* spawner, Actor* owner);
    ~CaixaAttack() override = default;

    std::vector<std::unique_ptr<Projectile>> Execute(const AttackParams& params) override;
};
