//
// Created by nslop on 18/11/2024.
//

#include <algorithm>
#include "Projectile.h"

#include <SDL_log.h>

#include "../Attacks/Behaviors.h"
#include "../Components/RigidBodyComponent.h"

Projectile::Projectile(Scene *scene, Actor *owner)
    : Actor(scene),
    mOwner(owner),
    mForwardSpeed(0.0f)
{

}

Projectile::~Projectile() = default;


void Projectile::OnUpdate(float deltaTime) {
    for (const auto &behavior: mBehaviors) {
        behavior->update(this, deltaTime);
    }

    mBehaviors.erase(
        std::remove_if(mBehaviors.begin(), mBehaviors.end(),
            [](const std::unique_ptr<ProjectileBehavior>& behavior) {
                return behavior->isFinished();
            }),
            mBehaviors.end());

    if (IsOffScreen()) {
        SetState(ActorState::Destroy);
    }
}

void Projectile::OnCollision(Actor *other) {

    // Comportamento Padrão. Battle só chama quando CheckCollisions dá true
    // Então sabemos que é uma colisão válida (Projetil do Jogador vs Boss, por exemplo)

    SetState(ActorState::Destroy);

}

float Projectile::GetForwardSpeed() const {
    if (auto rb = GetComponent<RigidBodyComponent>()) {
        return rb->GetVelocity().Length();
    }

    SDL_Log("Erro em Projectile.cpp na função GetForwardSpeed: Projetil sem RigidBodyComponent."
            "Retornando 0.0f em GetForwardSpeed...");
    return 0.0f;
}

void Projectile::SetForwardSpeed(const float newSpeed) {
    mForwardSpeed = newSpeed;
    if (auto rb = GetComponent<RigidBodyComponent>()) {
        Vector2 vel = rb->GetVelocity();
        vel.Normalize();
        rb->SetVelocity(vel * mForwardSpeed);
        return;
    }

    SDL_Log("Erro em Projectile.cpp na função SetForwardSpeed: Projetil sem RigidBodyComponent.");
}





