//
// Created by nslop on 17/12/2024.
//


#include "WaveAttack.h"
#include "../../Boss.h"
#include "../../../ProjectileFactory.h"

WaveAttack::WaveAttack(ProjectileFactory *pSpawner, Boss *owner, int numProjectiles,
                       float projectileSpeed, float angleBetweenExtremities, float centralAngle, float creationSpeed) :
                       mPSpawner(pSpawner),
                       mOwner(owner),
                       mNumProjectiles(numProjectiles),
                       mProjectileSpeed(projectileSpeed),
                       mAngle(angleBetweenExtremities),
                       mCentralAngle(centralAngle),
                       mCreationSpeed(creationSpeed)
{

}

std::vector<Projectile*> WaveAttack::execute() {

    std::vector<Projectile*> projectiles;
    projectiles.reserve(mNumProjectiles);

    auto angleStep = mAngle / (float) mNumProjectiles;
    auto startAngle = mCentralAngle - mAngle / 2;

    float pAngle, xCoordRad, yCoordRad;

    for(int i = 0; i < mNumProjectiles; i++) {
        auto p = mPSpawner->createProjectile(mOwner->GetScene(), mOwner);
        p->GetComponent<DrawAnimatedComponent>()->SetIsVisible(false);
        pAngle = i * angleStep + startAngle;
        xCoordRad = Math::Cos(pAngle * Math::Pi / 180);
        yCoordRad = Math::Sin(pAngle * Math::Pi / 180);
        auto directionVector = Vector2(xCoordRad, yCoordRad);
        p->insertBehavior<ActivateBehavior>(i * mCreationSpeed, directionVector * mProjectileSpeed);
        p->GetComponent<RigidBodyComponent>()->SetVelocity(Vector2::Zero);
        projectiles.push_back(p);
    }
    return projectiles;
}

std::vector<class Projectile *> WaveAttack::execute(int numProjectiles, float projectileSpeed,
        float angleBetweenExtremities, float centralAngle, float creationSpeed) {

    if(numProjectiles > 0) mNumProjectiles = numProjectiles;
    if(projectileSpeed != 0) mProjectileSpeed = projectileSpeed;
    if(angleBetweenExtremities > 0) mAngle = angleBetweenExtremities;
    SDL_Log("%f", mCentralAngle);
    if(centralAngle > 0) mCentralAngle = centralAngle;
    if(creationSpeed > 0) mCreationSpeed = creationSpeed;

    return execute();
}
