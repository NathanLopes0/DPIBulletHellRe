//
// Created by nslop on 18/11/2024.
//

#include <algorithm>
#include "Projectile.h"

#include <SDL_log.h>

#include "../Attacks/Behaviors.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/DrawComponents/DrawComponent.h"

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
        MarkDead();
    }
}

void Projectile::OnCollision(Actor *other) {

    // Comportamento Padrão. Battle só chama quando CheckCollisions dá true
    // Então sabemos que é uma colisão válida (Projetil do Jogador vs Boss, por exemplo)

    MarkDead();

}

void Projectile::Reset() {
    // Limpa TODOS os behaviors, inclusive os que não terminaram (ex: um
    // HomingBehavior com delay que nunca chegou a disparar porque o projétil
    // colidiu antes). Se não fizermos isso, um projétil reciclado herdaria
    // comportamento "fantasma" da vida anterior dele no pool.
    mBehaviors.clear();

    mForwardSpeed = 0.0f;

    if (auto rb = GetComponent<RigidBodyComponent>()) {
        rb->SetVelocity(Vector2::Zero);
        rb->SetAcceleration(Vector2::Zero);
    }

    // Reexibe o sprite, que MarkDead() escondeu quando este objeto foi
    // devolvido ao pool. Sem isso, o objeto reciclado ficaria invisível
    // mesmo depois de voltar a ser Active.
    if (auto draw = GetComponent<DrawComponent>()) {
        draw->SetIsVisible(true);
    }

    SetState(ActorState::Active);
}

void Projectile::MarkDead() {
    if (mOriginFactory) {
        // Vai voltar ao pool: esconde o sprite AGORA, porque o loop de
        // desenho (Game::GenerateOutput) itera Scene::mDrawables
        // incondicionalmente e não sabe nada sobre ActorState. Sem isto,
        // o objeto continuaria aparecendo, congelado, na posição onde
        // morreu, até a próxima reciclagem.
        if (auto draw = GetComponent<DrawComponent>()) {
            draw->SetIsVisible(false);
        }
        SetState(ActorState::Inactive);
    } else {
        // Sem factory de origem (ex: PlayerProjectile): morre de verdade,
        // como sempre foi. O destrutor do DrawComponent já cuida de
        // remover da lista de mDrawables da Scene.
        SetState(ActorState::Destroy);
    }
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





