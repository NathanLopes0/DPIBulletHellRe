//
// Created by nslop on 18/11/2024.
//

#include <algorithm>
#include "Projectile.h"

#include <SDL_log.h>

#include "../Attacks/Behaviors.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/DrawComponents/DrawComponent.h"
#include "../Components/ColliderComponents/ColliderComponent.h"

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

    // Religa o colisor que OnEnterPool() desligou ao guardar o objeto.
    if (auto collider = GetComponent<ColliderComponent>()) {
        collider->SetEnabled(true);
    }

    // Rotacao tambem e estado da vida anterior: LaserAttack chama SetRotation e
    // nada limpava isso, entao um projetil reciclado voltava torto.
    SetRotation(0.0f);

    // NOTA: a posicao NAO e resetada aqui de proposito - ver bug 8 da analise.
    // Toda strategy hoje chama SetPosition logo apos o Acquire, exceto quando a
    // condicao de firePosition falha. Isso sera tratado na proxima rodada.

    SetState(ActorState::Active);
}

void Projectile::Kill() {
    MarkDead();
}

void Projectile::OnEnterPool() {
    // Um objeto guardado no pool CONTINUA registrado em Scene::mDrawables: o
    // DrawComponent so se desregistra no proprio destrutor, e o objeto nao e
    // destruido. Como Game::GenerateOutput percorre mDrawables sem consultar
    // ActorState, este mIsVisible = false e a UNICA coisa que impede um objeto
    // guardado de continuar aparecendo na tela.
    if (auto draw = GetComponent<DrawComponent>()) {
        draw->SetIsVisible(false);
    }

    // Desliga o colisor enquanto o objeto esta parado no pool.
    if (auto collider = GetComponent<ColliderComponent>()) {
        collider->SetEnabled(false);
    }

    if (auto rb = GetComponent<RigidBodyComponent>()) {
        rb->SetVelocity(Vector2::Zero);
        rb->SetAcceleration(Vector2::Zero);
    }

    // Behaviors nao terminados nao podem sobreviver ate a proxima vida.
    mBehaviors.clear();

    SetState(ActorState::Inactive);
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





