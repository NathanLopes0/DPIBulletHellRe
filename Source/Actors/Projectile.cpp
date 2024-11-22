//
// Created by nslop on 18/11/2024.
//

#include "Projectile.h"
#include "../Game.h"


Projectile::Projectile(Scene *scene) :
        Actor(scene)
{
    mOwner = nullptr;
    mDrawComponent = nullptr;
    mColliderComponent = new CircleColliderComponent(this, 12);
    mRigidBodyComponent = new RigidBodyComponent(this);

}

void Projectile::OnUpdate(float deltaTime) {
    if(!InsideProjectileLimit()) {
        DeactivateProjectile();
    }

}

void Projectile::OnCollision() {

}

void Projectile::DeactivateProjectile() {

}

// Ativa o Projétil na posição passada como parâmetro
void Projectile::ActivateProjectile(const Vector2& desiredStartingPosition) {
    SetState(ActorState::Active);
    mDrawComponent->SetIsVisible(true);

    SetPosition(desiredStartingPosition);
}
