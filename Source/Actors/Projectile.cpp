//
// Created by nslop on 18/11/2024.
//


#include <memory>
#include "../Game.h"
#include "Projectile.h"


Projectile::Projectile(Scene *scene) :
        Actor(scene)
{
    mDrawComponent = nullptr;
    mColliderComponent = new CircleColliderComponent(this, 12);
    mRigidBodyComponent = new RigidBodyComponent(this);

}

void Projectile::OnUpdate(float deltaTime) {
    if(!InsideProjectileLimit()) {
        DestroyProjectile();
    }

}

void Projectile::OnCollision() {

}

void Projectile::DeactivateProjectile() {
    mDrawComponent->SetIsVisible(false);
    SetState(ActorState::Paused);
}

void Projectile::ActivateProjectile() {
    mDrawComponent->SetIsVisible(true);
    SetState(ActorState::Active);
}
// Ativa o Projétil na posição passada como parâmetro
void Projectile::ActivateProjectile(const Vector2& desiredStartingPosition) {
    ActivateProjectile();
    SetPosition(desiredStartingPosition);
}

float Projectile::GetFowardSpeed() {
    return mRigidBodyComponent->GetVelocity().Length();
}

void Projectile::DestroyProjectile() {
    SetState(ActorState::Destroy);
}
