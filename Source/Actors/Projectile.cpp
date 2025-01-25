//
// Created by nslop on 18/11/2024.
//


#include <memory>
#include "../Game.h"
#include "Projectile.h"


Projectile::Projectile(Scene *scene) :
        Actor(scene)
{
    mRigidBodyComponent = new RigidBodyComponent(this);
}

void Projectile::OnUpdate(float deltaTime) {
    if(!InsideProjectileLimit()) {
        DestroyProjectile();
    }

}

void Projectile::OnCollision() {

}

// Ativa o Projétil na posição passada como parâmetro
void Projectile::ActivateProjectile(const Vector2& desiredStartingPosition) {
    GetComponent<DrawAnimatedComponent>()->SetIsVisible(true);
    SetState(ActorState::Active);
    SetPosition(desiredStartingPosition);
}

float Projectile::GetFowardSpeed() {
    return mRigidBodyComponent->GetVelocity().Length();
}

void Projectile::DestroyProjectile() {
    SetState(ActorState::Destroy);
    Kill();
}
