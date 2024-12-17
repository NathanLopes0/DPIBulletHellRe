//
// Created by nslop on 17/12/2024.
//

#ifndef DPIBULLETHELLRE_WAVEATTACK_H
#define DPIBULLETHELLRE_WAVEATTACK_H


#include "../../../../AttackStrategy.h"

class WaveAttack : public AttackStrategy {
public:

    WaveAttack(class ProjectileFactory* pSpawner, class Boss* owner, int numProjectiles,
               float projectileSpeed, float angleBetweenExtremities, float centralAngle,
               float creationSpeed);

    std::vector<class Projectile*> execute() override;
    std::vector<class Projectile*> execute(int, float, float, float, float) override;

private:

    class ProjectileFactory* mPSpawner;
    class Boss* mOwner;
    int mNumProjectiles;
    float mProjectileSpeed, mAngle, mCentralAngle, mCreationSpeed;
};


#endif //DPIBULLETHELLRE_WAVEATTACK_H
