//
// Created by gensh on 20/03/2026.
//

#ifndef DPIBULLETHELLRE_BALOONATTACK_H
#define DPIBULLETHELLRE_BALOONATTACK_H
#include "../IAttackStrategy.h"


class BaloonAttack : public IAttackStrategy {

public:
    BaloonAttack(ProjectileFactory* projectileFactory, Actor* owner);
    ~BaloonAttack() override = default;

    std::vector<std::unique_ptr<Projectile>> Execute(const AttackParams& params) override;

private:

    ProjectileFactory* mSpawner;
    Actor* mOwner;

};


#endif //DPIBULLETHELLRE_BALOONATTACK_H