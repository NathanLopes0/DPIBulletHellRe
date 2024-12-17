//
// Created by nslop on 16/12/2024.
//

#include "../../Boss.h"
#include "AngledFilledAttack.h"
#include "../../../ProjectileFactory.h"

AngledFilledAttack::AngledFilledAttack(ProjectileFactory *pSpawner, Boss *owner, int numProjectiles,
                                       float projectileSpeed, float angleBetweenExtremities, float centralAngle) {

    mPSpawner = pSpawner;
    mOwner = owner;
    mNumProjectiles = numProjectiles;
    mProjectileSpeed = projectileSpeed;
    mAngle = angleBetweenExtremities;
    mCentralAngle = centralAngle;

}

std::vector<Projectile*> AngledFilledAttack::execute() {
    std::vector<Projectile*> projectiles;

    auto angleStep = mAngle/(float)mNumProjectiles;
    auto startAngle = mCentralAngle - mAngle/2;

    float pAngle, xCoordRad, yCoordRad;

    for(int i = 0; i < mNumProjectiles; i++) {
        auto p = mPSpawner->createProjectile(mOwner->GetScene(), mOwner);
        pAngle = i * angleStep + startAngle;
        xCoordRad = Math::Cos(pAngle * Math::Pi / 180);
        yCoordRad = Math::Sin(pAngle * Math::Pi / 180);
        auto directionVector = Vector2(xCoordRad, yCoordRad);
        p->GetComponent<RigidBodyComponent>()->SetVelocity(directionVector * mProjectileSpeed);
        projectiles.push_back(p);
    }

    return projectiles;
}

std::vector<class Projectile *>
AngledFilledAttack::execute(int numProjectiles, float projectileSpeed, float angleBetweenExtremities,
                            float centralAngle) {
    if(numProjectiles > 0) mNumProjectiles = numProjectiles;
    if(projectileSpeed != 0) mProjectileSpeed = projectileSpeed;
    if(angleBetweenExtremities > 0) mAngle = angleBetweenExtremities;
    if(centralAngle > 0) mCentralAngle = centralAngle;
    return execute();
}


AngledFilledAttack::~AngledFilledAttack() = default;
