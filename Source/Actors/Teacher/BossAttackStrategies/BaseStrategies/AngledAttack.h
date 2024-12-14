//
// Created by nslop on 12/12/2024.
//

#ifndef DPIBULLETHELLRE_ANGLEDATTACK_H
#define DPIBULLETHELLRE_ANGLEDATTACK_H

#include "../../../../AttackStrategy.h"

class AngledAttack : public AttackStrategy {
public:
    [[maybe_unused]] AngledAttack(class ProjectileFactory* spawner, class Boss* owner,
            float _projectileSpeed, float _angleBetween, float _centralAngle);

    ~AngledAttack();

    std::vector<class Projectile*> execute() override;

private:
    class ProjectileFactory* mPSpawner;
    class Boss* mOwner;
    float mProjectileSpeed;
    float mAngle;
    float mCentralAngle;
};


#endif //DPIBULLETHELLRE_ANGLEDATTACK_H
