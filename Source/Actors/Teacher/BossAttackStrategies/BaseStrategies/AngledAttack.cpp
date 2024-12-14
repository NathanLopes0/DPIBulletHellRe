//
// Created by nslop on 12/12/2024.
//


#include "../../Boss.h"
#include "AngledAttack.h"
#include "../../../ProjectileFactory.h"


[[maybe_unused]] AngledAttack::AngledAttack(ProjectileFactory *spawner, Boss *owner,  float _projectileSpeed,
                           float _angleBetween, float _centralAngle) {
    mPSpawner = spawner;
    mOwner = owner;
    mProjectileSpeed = _projectileSpeed;
    mAngle = _angleBetween;
    mCentralAngle = _centralAngle;
}

std::vector<Projectile*> AngledAttack::execute() {
    std::vector<Projectile*> projectiles;

    auto p1 = mPSpawner->createProjectile(mOwner->GetScene(), mOwner);
    auto p2 = mPSpawner->createProjectile(mOwner->GetScene(), mOwner);

    auto p1Angle = mCentralAngle - mAngle/2;
    auto p2Angle = mCentralAngle + mAngle/2;

    float xCoordRadP1 = Math::Cos(p1Angle * Math::Pi / 180);
    float yCoordRadP1 = Math::Sin(p1Angle * Math::Pi / 180);
    float xCoordRadP2 = Math::Cos(p2Angle * Math::Pi / 180);
    float yCoordRadP2 = Math::Sin(p2Angle * Math::Pi / 180);

    auto directionVectorP1 = Vector2(xCoordRadP1, yCoordRadP1);
    auto directionVectorP2 = Vector2(xCoordRadP2, yCoordRadP2);

    p1->GetComponent<RigidBodyComponent>()->SetVelocity(directionVectorP1 * mProjectileSpeed);
    p2->GetComponent<RigidBodyComponent>()->SetVelocity(directionVectorP2 * mProjectileSpeed);

    projectiles.push_back(p1);
    projectiles.push_back(p2);

    return projectiles;
}

AngledAttack::~AngledAttack() = default;
