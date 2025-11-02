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
    : mSpawner(spawner),
      mOwner(owner)
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

    float angleStep = 0.0f;
    if (numProjectiles > 1) {
        angleStep = totalAngle / static_cast<float>(numProjectiles) - 1.0f;
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


    // --- 1. Criação (via Factory) ---
    auto projectile = mSpawner->createProjectile(mOwner->GetScene(), mOwner);
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
    projectile->GetComponent<RigidBodyComponent>()->SetVelocity(directionVector * params.projectileSpeed);

    return projectile;
}
