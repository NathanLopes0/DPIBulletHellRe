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
// TrackingBehavior
// ---------------------------------------------------------------------------
void TrackingBehavior::update(Projectile* p, float deltaTime) {

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

    // Decaimento linear: a forca cai de trackingStrength ate 0 ao longo de
    // trackingTime. Quando zera, o projetil segue reto e o behavior sai da
    // lista (isFinished).
    const float t = elapsedTime - startDelay;
    if (trackingTime <= 0.0f || t >= trackingTime) {
        finished = true;
        return;
    }
    const float forca = trackingStrength * (1.0f - t / trackingTime);

    const Vector2 velocity = rb->GetVelocity();
    const float speed = velocity.Length();

    // Sem velocidade nao ha direcao a corrigir. Sair daqui tambem evita
    // normalizar um vetor nulo.
    if (Math::NearZero(speed)) return;

    // Passo de correcao neste frame, limitado a 1.0 para nunca ultrapassar a
    // direcao desejada (o que faria o projetil oscilar em vez de convergir).
    float step = forca * deltaTime;
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
// WobbleBehavior
// ---------------------------------------------------------------------------
void WobbleBehavior::update(Projectile* p, float deltaTime) {

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

// ---------------------------------------------------------------------------
// PathBehavior
// ---------------------------------------------------------------------------
void PathBehavior::update(Projectile* p, float deltaTime) {

    auto rb = p->GetComponent<RigidBodyComponent>();
    if (!rb || waypoints.empty()) {
        finished = true;
        return;
    }

    elapsedTime += deltaTime;
    if (elapsedTime < startDelay) return;

    // Captura posicao e orientacao UMA vez, na ativacao. Tudo depois disso e
    // calculado relativo a este instante.
    if (!started) {
        origin = p->GetPosition();

        const Vector2 v = rb->GetVelocity();
        const float speed = v.Length();

        if (!Math::NearZero(speed)) {
            // Alinha o caminho com a direcao de viagem: +X do espaco de
            // caminho passa a apontar para onde o projetil ja ia.
            cosR = v.x / speed;
            sinR = v.y / speed;
            if (pathSpeed <= 0.0f) pathSpeed = speed;
        } else {
            // Sem velocidade nao ha direcao a alinhar: usa o caminho como foi
            // escrito. pathSpeed precisa ser positivo ou o projetil ficaria
            // parado para sempre, sem nunca sair da tela nem voltar ao pool.
            cosR = 1.0f;
            sinR = 0.0f;
            if (pathSpeed <= 0.0f) pathSpeed = 100.0f;
        }

        started = true;
    }

    // Waypoint atual, convertido de espaco de caminho para espaco de mundo.
    const Vector2& wp = waypoints[current];
    const Vector2 target(origin.x + wp.x * cosR - wp.y * sinR,
                         origin.y + wp.x * sinR + wp.y * cosR);

    Vector2 toTarget = target - p->GetPosition();
    const float distance = toTarget.Length();

    // Tolerancia proporcional ao passo do frame. Com um valor fixo pequeno, um
    // projetil rapido passa por cima do waypoint sem nunca entrar no raio de
    // chegada e fica orbitando em torno dele indefinidamente.
    float tolerance = pathSpeed * deltaTime * 1.5f;
    if (tolerance < 2.0f) tolerance = 2.0f;

    if (distance <= tolerance) {
        ++current;

        if (current >= waypoints.size()) {
            // Fim do caminho: encerra e deixa a velocidade atual valendo, entao
            // o projetil segue reto. Zerar a velocidade aqui criaria um
            // projetil imortal, parado na tela e nunca devolvido ao pool.
            finished = true;
        }
        return;
    }

    toTarget.Normalize();
    rb->SetVelocity(toTarget * pathSpeed);
}

// ---------------------------------------------------------------------------
// PathBehavior
// ---------------------------------------------------------------------------
void PathBehavior::update(Projectile* p, float deltaTime) {

    auto rb = p->GetComponent<RigidBodyComponent>();
    if (!rb || waypoints.empty()) {
        finished = true;
        return;
    }

    elapsedTime += deltaTime;
    if (elapsedTime < startDelay) return;

    // Captura posicao e orientacao UMA vez, na ativacao. Tudo depois disso e
    // calculado relativo a este instante.
    if (!started) {
        origin = p->GetPosition();

        const Vector2 v = rb->GetVelocity();
        const float speed = v.Length();

        if (!Math::NearZero(speed)) {
            // Alinha o caminho com a direcao de viagem: +X do espaco de
            // caminho passa a apontar para onde o projetil ja ia.
            cosR = v.x / speed;
            sinR = v.y / speed;
            if (pathSpeed <= 0.0f) pathSpeed = speed;
        } else {
            // Sem velocidade nao ha direcao a alinhar: usa o caminho como foi
            // escrito. pathSpeed precisa ser positivo ou o projetil ficaria
            // parado para sempre, sem nunca sair da tela nem voltar ao pool.
            cosR = 1.0f;
            sinR = 0.0f;
            if (pathSpeed <= 0.0f) pathSpeed = 100.0f;
        }

        started = true;
    }

    // Waypoint atual, convertido de espaco de caminho para espaco de mundo.
    const Vector2& wp = waypoints[current];
    const Vector2 target(origin.x + wp.x * cosR - wp.y * sinR,
                         origin.y + wp.x * sinR + wp.y * cosR);

    Vector2 toTarget = target - p->GetPosition();
    const float distance = toTarget.Length();

    // Tolerancia proporcional ao passo do frame. Com um valor fixo pequeno, um
    // projetil rapido passa por cima do waypoint sem nunca entrar no raio de
    // chegada e fica orbitando em torno dele indefinidamente.
    float tolerance = pathSpeed * deltaTime * 1.5f;
    if (tolerance < 2.0f) tolerance = 2.0f;

    if (distance <= tolerance) {
        ++current;

        if (current >= waypoints.size()) {
            // Fim do caminho: encerra e deixa a velocidade atual valendo, entao
            // o projetil segue reto. Zerar a velocidade aqui criaria um
            // projetil imortal, parado na tela e nunca devolvido ao pool.
            finished = true;
        }
        return;
    }

    toTarget.Normalize();
    rb->SetVelocity(toTarget * pathSpeed);
}
