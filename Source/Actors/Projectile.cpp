//
// Created by nslop on 18/11/2024.
//


#include <memory>
#include "../Game.h"
#include "Projectile.h"
#include "Teacher/BossAttackStrategies/Behaviors.h"


Projectile::Projectile(Scene *scene) :
        Actor(scene)
{
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

float Projectile::GetFowardSpeed() {
    if(mRigidBodyComponent)
        SDL_Log("%f", mRigidBodyComponent->GetVelocity().Length());
    return mRigidBodyComponent->GetVelocity().Length();
}
