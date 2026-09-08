//
// Created by gensh on 17/10/2025.
//


#include "Behaviors.h"
#include "../Components/RigidBodyComponent.h"
#include "../Actors/Teachers/Bosses/BossesProjectiles/BossProjectile.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"

void HomingBehavior::update(Projectile* p, float deltaTime) {

    if (auto bossProj = dynamic_cast<BossProjectile*>(p)) {
        if(!homing) {
            elapsedTime += deltaTime;
            if(elapsedTime >= homingDelay) {
                homing = true;
                if(homingSpeed == 0) {
                    auto currSpeed = bossProj->GetForwardSpeed();
                    bossProj->GetComponent<RigidBodyComponent>()->SetVelocity(
                                bossProj->GetPlayerDirection() * currSpeed);
                }
                else {
                    bossProj->GetComponent<RigidBodyComponent>()->SetVelocity(
                                bossProj->GetPlayerDirection() * homingSpeed);
                }
            }
        }
    }
}
void AccelerateBehavior::update(Projectile* p, float deltaTime) {
    if (auto bossProj = dynamic_cast<BossProjectile*>(p)) {
        if(!accelerated) {
            elapsedTime += deltaTime;
            if(elapsedTime >= accelerateDelay) {
                accelerated = true;
                auto currVel = bossProj->GetComponent<RigidBodyComponent>()->GetVelocity();
                auto currVelAux = currVel;
                currVel.Normalize();
                if(accelerateSpeedValue != -1) {
                    currVel *= static_cast<float>(accelerateSpeedValue);
                    bossProj->GetComponent<RigidBodyComponent>()->SetVelocity(currVelAux + currVel);
                }
                else {
                    accelerateSpeedPercent = Math::Max(accelerateSpeedPercent, 1.f);
                    bossProj->GetComponent<RigidBodyComponent>()->SetVelocity(currVelAux * accelerateSpeedPercent);
                }
            }
        }
    }
}
void SlowDownBehavior::update(Projectile* p, float deltaTime) {
    if (auto bossProj = dynamic_cast<BossProjectile*>(p)) {
        if(!slowedDown) {
            elapsedTime += deltaTime;
            if(elapsedTime >= slowdownDelay) {
                slowedDown = true;
                auto currVel = p->GetComponent<RigidBodyComponent>()->GetVelocity();
                auto currVelAux = currVel;
                currVel.Normalize();
                if(slowdownSpeedValue != -1) {
                    currVel *= static_cast<float>(slowdownSpeedValue);
                    bossProj->GetComponent<RigidBodyComponent>()->SetVelocity(currVelAux - currVel);
                }
                else {
                    Math::Clamp(slowdownSpeedPercent, 0.f, 1.f);
                    bossProj->GetComponent<RigidBodyComponent>()->SetVelocity(currVelAux * slowdownSpeedPercent);
                }
            }
        }
    }
}
void ActivateBehavior::update(Projectile* p, float deltaTime) {
    if (auto bossProj = dynamic_cast<BossProjectile*>(p)) {
        if(!activated) {
            elapsedTime += deltaTime;
            if(elapsedTime >= activationDelay) {
                p->GetComponent<DrawAnimatedComponent>()->SetIsVisible(true);
                p->GetComponent<RigidBodyComponent>()->SetVelocity(activationVelocity);
                activated = true;
            }
        }
    }
}

void DeactivateBehavior::update(Projectile *p, const float deltaTime) {
    if (auto bossProj = dynamic_cast<BossProjectile*>(p)) {
        if (!deactivated) {
            elapsedTime += deltaTime;
            if (elapsedTime >= deactivationDelay) {
                p->GetComponent<DrawAnimatedComponent>()->SetIsVisible(false);
                p->GetComponent<RigidBodyComponent>()->SetVelocity(Vector2::Zero);
                deactivated = true;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// GradientDescentBehavior
// ---------------------------------------------------------------------------
void GradientDescentBehavior::update(Projectile* p, float deltaTime) {

    // Precisa saber onde o jogador esta, e so BossProjectile sabe.
    auto bossProj = dynamic_cast<BossProjectile*>(p);
    if (!bossProj) {
        finished = true;
        return;
    }

    elapsedTime += deltaTime;
    if (elapsedTime < startDelay) return;

    auto rb = bossProj->GetComponent<RigidBodyComponent>();
    if (!rb) {
        finished = true;
        return;
    }

    // Annealing linear: a taxa cai de learningRate ate 0 ao longo de
    // convergenceTime. Quando zera, o projetil segue reto e o behavior sai da
    // lista (isFinished).
    const float t = elapsedTime - startDelay;
    if (convergenceTime <= 0.0f || t >= convergenceTime) {
        finished = true;
        return;
    }
    const float rate = learningRate * (1.0f - t / convergenceTime);

    const Vector2 velocity = rb->GetVelocity();
    const float speed = velocity.Length();

    // Sem velocidade nao ha direcao a corrigir. Sair daqui tambem evita
    // normalizar um vetor nulo.
    if (Math::NearZero(speed)) return;

    // Passo de correcao neste frame, limitado a 1.0 para nunca ultrapassar a
    // direcao desejada (o que faria o projetil oscilar em vez de convergir).
    float step = rate * deltaTime;
    if (step > 1.0f) step = 1.0f;

    const Vector2 desired = bossProj->GetPlayerDirection() * speed;
    Vector2 corrected = velocity + (desired - velocity) * step;

    // Se a correcao anulou o vetor (caso raro: velocidade exatamente oposta a
    // desejada com step 0.5), manter a velocidade anterior. Sem esta guarda o
    // projetil pararia, nunca sairia da tela e nunca voltaria para o pool.
    if (Math::NearZero(corrected.Length())) return;

    corrected.Normalize();
    rb->SetVelocity(corrected * speed);
}

// ---------------------------------------------------------------------------
// OverfitBehavior
// ---------------------------------------------------------------------------
void OverfitBehavior::update(Projectile* p, float deltaTime) {

    auto rb = p->GetComponent<RigidBodyComponent>();
    if (!rb) {
        finished = true;
        return;
    }

    elapsedTime += deltaTime;
    if (elapsedTime < startDelay) return;

    // Captura a direcao base UMA unica vez. Reler a velocidade todo frame faria
    // a oscilacao realimentar a si mesma e o projetil sairia em espiral.
    if (!started) {
        const Vector2 velocity = rb->GetVelocity();
        baseSpeed = velocity.Length();

        if (Math::NearZero(baseSpeed)) {
            // Projetil parado na ativacao: nao ha eixo para oscilar em torno.
            finished = true;
            return;
        }

        baseDirection = velocity;
        baseDirection.Normalize();
        started = true;
    }

    const float t = elapsedTime - startDelay;
    if (duration <= 0.0f || t >= duration) {
        // Termina alinhado com a direcao base, sem desvio residual.
        rb->SetVelocity(baseDirection * baseSpeed);
        finished = true;
        return;
    }

    // Amplitude decai linearmente; a fase avanca com a frequencia dada.
    const float decay = 1.0f - t / duration;
    const float angleDeg = maxAngle * decay * Math::Sin(2.0f * Math::Pi * frequency * t);
    const float angleRad = Math::ToRadians(angleDeg);

    // Rotaciona a direcao base pelo desvio atual.
    const float c = Math::Cos(angleRad);
    const float s = Math::Sin(angleRad);
    const Vector2 rotated(baseDirection.x * c - baseDirection.y * s,
                          baseDirection.x * s + baseDirection.y * c);

    rb->SetVelocity(rotated * baseSpeed);
}
