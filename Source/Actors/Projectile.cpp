//
// Created by nslop on 18/11/2024.
//

#include <algorithm>
#include "Projectile.h"

#include <SDL_log.h>

#include "../Attacks/Behaviors.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/DrawComponents/DrawComponent.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Components/ColliderComponents/ColliderComponent.h"

Projectile::Projectile(Scene *scene, Actor *owner)
    : Actor(scene),
    mOwner(owner)
{
}

Projectile::~Projectile() = default;

void Projectile::OnUpdate(float deltaTime) {

    // Ordem importa: a Motion escreve a velocidade, os Modifiers ajustam o que ela escreveu.
    if (mMotion) {
        mMotion->update(this, deltaTime);
        if (mMotion->isFinished()) {
            mMotion.reset();
        }
    }

    for (const auto& modifier : mModifiers) {
        modifier->update(this, deltaTime);
    }

    mModifiers.erase(
        std::remove_if(mModifiers.begin(), mModifiers.end(),
            [](const std::unique_ptr<ProjectileBehavior>& m) {
                return m->isFinished();
            }),
            mModifiers.end());

    mTempoDeVida += deltaTime;

    if (IsOffScreen()) {
        MarkDead();
    }
    else if (mTempoDeVida >= kTempoDeVidaMaximo) {
        // Nao deveria acontecer em jogo.
        SDL_Log("Projetil vivo ha %.0fs sem sair da tela (|v| = %.0f px/s). "
                "Encerrado pelo tempo maximo de vida.",
                mTempoDeVida,
                GetComponent<RigidBodyComponent>() ? GetComponent<RigidBodyComponent>()->GetVelocity().Length() : 0.f);
        MarkDead();
    }
}

void Projectile::OnCollision(Actor *other) {

    // Comportamento Padrão.

    MarkDead();

}

void Projectile::MarcarPadraoDeFabrica() {

    mEscalaPadrao = GetScale();

    if (const auto anim = GetComponent<DrawAnimatedComponent>()) {
        mAnimacaoPadrao = anim->GetAnimation();
    }

    mPadraoMarcado = true;
}

void Projectile::Reset() {
    // Limpa todos os behaviors, inclusive os que não terminaram (ex.
    mMotion.reset();
    mModifiers.clear();
    mTempoDeVida = 0.0f;

    if (auto rb = GetComponent<RigidBodyComponent>()) {
        rb->SetVelocity(Vector2::Zero);
        rb->SetAcceleration(Vector2::Zero);
    }

    // Reexibe o sprite, que MarkDead() escondeu quando este objeto foi devolvido ao pool.
    if (auto draw = GetComponent<DrawComponent>()) {
        draw->SetIsVisible(true);
    }

    // Religa o colisor que OnEnterPool() desligou ao guardar o objeto.
    if (auto collider = GetComponent<ColliderComponent>()) {
        collider->SetEnabled(true);
    }

    // Rotacao tambem e estado da vida anterior.
    SetRotation(0.0f);

    // Devolve a aparencia de fabrica.
    if (mPadraoMarcado) {
        SetScale(mEscalaPadrao);

        if (!mAnimacaoPadrao.empty()) {
            if (const auto anim = GetComponent<DrawAnimatedComponent>()) {
                anim->SetAnimation(mAnimacaoPadrao);
            }
        }
    }

    // Nota: a posicao nao e resetada aqui - ver bug 8 da analise.

    SetState(ActorState::Active);
}

void Projectile::Kill() {
    MarkDead();
}

void Projectile::OnEnterPool() {
    // Um objeto guardado no pool continua registrado em Scene::mDrawables.
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
    mMotion.reset();
    mModifiers.clear();
    mTempoDeVida = 0.0f;

    SetState(ActorState::Inactive);
}

void Projectile::MarkDead() {
    if (mOriginFactory) {
        if (auto draw = GetComponent<DrawComponent>()) {
            draw->SetIsVisible(false);
        }
        SetState(ActorState::Inactive);
    } else {
        // O destrutor do DrawComponent já cuida de remover da lista de mDrawables da Scene.
        SetState(ActorState::Destroy);
    }
}

