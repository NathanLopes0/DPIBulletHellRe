//
// Created by nslop on 12/12/2024.
//

#include <memory>
#include "SDL_log.h"
#include "AngledAttack.h"
#include "../../Actors/Actor.h"
#include "../../Actors/Projectile.h"
#include "../../Actors/ProjectileFactory.h"
#include "../AttackParameters/AttackParams.h"
#include "../../Components/RigidBodyComponent.h"

AngledAttack::AngledAttack(ProjectileFactory* spawner, Actor* owner)
    : IAttackStrategy(spawner, owner)
{

}

std::vector<std::unique_ptr<Projectile>> AngledAttack::Execute(const AttackParams& params) {
    std::vector<std::unique_ptr<Projectile>> projectiles;

    // Lendo a configuração que precisamos do params
    const int numProjectiles = params.numProjectiles;
    const float totalAngle = params.angle;           // O ângulo total do "leque"
    const float centralAngle = params.centralAngle;  // A direção central do "leque"

    // Failsafe: Se não há projéteis, não há o que fazer.
    if (numProjectiles == 0) {
        return projectiles;
    }

    projectiles.reserve(numProjectiles);

    // Angulo inicial
    const float angleStart = centralAngle - totalAngle / 2.0f;

    // Parenteses: o passo tem de ser totalAngle / (n - 1), nao
    // (totalAngle / n) - 1. Com n intervalos entre n+1 projeteis o leque nunca
    // fecharia; com n-1 intervalos entre n projeteis, o primeiro cai em
    // angleStart e o ultimo exatamente em angleStart + totalAngle.
    // A versao antiga encolhia o leque e, para n grande, o passo virava
    // NEGATIVO (n=1000, angulo=30 dava passo -0.97 e varria -894 graus).
    // WaveAttack ja usava a formula correta; agora as duas batem.
    float angleStep = 0.0f;
    if (numProjectiles > 1) {
        angleStep = totalAngle / (static_cast<float>(numProjectiles) - 1.0f);
    }

    for (int i = 0; i < numProjectiles; i++) {

        // Calcula o ângulo para este projétil específico
        float currAngle = angleStart + (static_cast<float>(i) * angleStep);

        if (auto projectile = CreateProjectileAtAngle(params, currAngle)) {
            projectiles.push_back(std::move(projectile));
        }

    }

    return projectiles;
}


/**
 * @brief Função auxiliar (Construtora).
 * Cria e configura um único projétil.
 */
std::unique_ptr<Projectile> AngledAttack::CreateProjectileAtAngle(const AttackParams& params, const float angleInDegrees) {


    // --- 1. Criação (via Factory, com Object Pooling) ---
    auto projectile = Acquire();
    if (!projectile) {
        SDL_Log("Erro em AngledAttack.cpp - CreateProjectileAtAngle: falha em criar projétil");
        return nullptr;
    }

    // --- 2. Cálculo de Direção (Matemática Pura) ---
    const float xCoordRad = Math::Cos(angleInDegrees * Math::Pi / 180);
    const float yCoordRad = Math::Sin(angleInDegrees * Math::Pi / 180);
    Vector2 directionVector(xCoordRad, yCoordRad);
    directionVector.Normalize();

    // --- 3. Configuração ---
    projectile->SetPosition(params.firePosition);

    // Guarda de nulo: antes o RigidBodyComponent era dereferenciado direto.
    auto rb = projectile->GetComponent<RigidBodyComponent>();
    if (!rb) {
        SDL_Log("Erro em AngledAttack: projetil sem RigidBodyComponent, descartado.");
        return nullptr;
    }
    rb->SetVelocity(directionVector * params.projectileSpeed);

    return projectile;
}
