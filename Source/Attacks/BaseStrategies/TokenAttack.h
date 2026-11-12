//
// A geracao de tokens: um por vez, sorteados com temperatura.
//

#pragma once

#include <cstddef>

#include "../IAttackStrategy.h"

/// A ideia.
class TokenAttack : public IAttackStrategy {
public:
    TokenAttack(ProjectileFactory* spawner, Actor* owner);
    ~TokenAttack() override = default;

    std::vector<std::unique_ptr<Projectile>> Execute(const AttackParams& params) override;

private:
    std::size_t mProximoToken = 0;
};
