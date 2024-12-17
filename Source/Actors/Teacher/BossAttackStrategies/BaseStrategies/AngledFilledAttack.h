//
// Created by nslop on 16/12/2024.
//

#ifndef DPIBULLETHELLRE_ANGLEDFILLEDATTACK_H
#define DPIBULLETHELLRE_ANGLEDFILLEDATTACK_H


#include "../../../../AttackStrategy.h"

class AngledFilledAttack : public AttackStrategy {
public:
    AngledFilledAttack(class ProjectileFactory* pSpawner, class Boss* owner, int numProjectiles,
            float projectileSpeed, float angleBetweenExtremities, float centralAngle);
    ~AngledFilledAttack();

    std::vector<class Projectile*> execute() override;
    std::vector<class Projectile*> execute(int numProjectiles, float projectileSpeed,
            float angleBetweenExtremities, float centralAngle) override;

private:
    class ProjectileFactory* mPSpawner;
    class Boss* mOwner;
    int mNumProjectiles;
    float mProjectileSpeed;
    float mAngle;
    float mCentralAngle;

};


#endif //DPIBULLETHELLRE_ANGLEDFILLEDATTACK_H
