//
// Created by nslop on 26/11/2024.
//


#include "../../Actors/Teacher/Boss.h"
#include "CircularSpreadStrategy.h"
#include "../../../ProjectileFactory.h"

CircularSpreadAttack::CircularSpreadAttack(ProjectileFactory* factory, Boss* owner, int _numProjectiles, float _projectileSpeed)
{
    numProjectiles = _numProjectiles;
    projectileSpeed = _projectileSpeed;
    mPSpawner = factory;
    mOwner = owner;

}

std::vector<Projectile *> CircularSpreadAttack::execute() {

    std::vector<Projectile*> projectiles;

    float angleStep = 360.0f / (float)numProjectiles; //angulo entre cada um dos projéteis do círculo

    for(int i = 0; i < numProjectiles; i++) {

        // --------------------- CRIAÇÃO E LÓGICA DE CADA PROJÉTIL -------------------------- //
        auto projectile = mPSpawner->createProjectile(mOwner->GetScene(), mOwner);
        projectile->SetPosition(mOwner->GetPosition());

        float xCoordRad = Math::Cos(i * angleStep * Math::Pi / 180);
        float yCoordRad = Math::Sin(i * angleStep * Math::Pi / 180);
        Vector2 directionVector = Vector2(xCoordRad, yCoordRad);
        directionVector.Normalize();

        projectile->GetComponent<RigidBodyComponent>()->SetVelocity(directionVector * projectileSpeed);


        projectiles.push_back(projectile);
    }

    return projectiles;
}

CircularSpreadAttack::~CircularSpreadAttack() = default;

