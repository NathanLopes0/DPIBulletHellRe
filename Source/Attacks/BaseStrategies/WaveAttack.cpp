//
// Created by nslop on 17/12/2024.
//

#include "WaveAttack.h"
#include "../AttackParameters/AttackParams.h"
#include "../../Actors/Projectile.h"  // Para insertBehavior
#include "../Behaviors.h"     // Para pegar o ActivateBehavior
#include "../../Actors/ProjectileFactory.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include <memory>

WaveAttack::WaveAttack(ProjectileFactory* spawner, Actor* owner)
    : mSpawner(spawner), mOwner(owner)
{

}

std::vector<std::unique_ptr<Projectile>> WaveAttack::Execute(const AttackParams& params) {

    std::vector<std::unique_ptr<Projectile>> projectiles;

    // Lemos a configuração que precisamos do params
    const int numProjectiles = params.numProjectiles;
    const float projectileSpeed = params.projectileSpeed;
    const float totalAngle = params.angle;
    const float centralAngle = params.centralAngle;
    const float creationSpeed = params.creationSpeed; // O "atraso" da onda
    ProjectileFactory* spawner = mSpawner;
    Actor* owner = mOwner;

    if (!spawner || !owner) {
        SDL_Log("sem spawner ou sem owner");
        return {};
    }

    if (numProjectiles == 0) {
        return projectiles;
    }
    projectiles.reserve(numProjectiles);

    // Mesma matemática de "leque" do AngledAttack
    const float angleStart = centralAngle - totalAngle / 2.0f;

    float angleStep = 0.0f;
    if (numProjectiles > 1) {
        angleStep = totalAngle / (static_cast<float>(numProjectiles) - 1.0f);
    }

    // O Loop de N Projéteis
    for (int i = 0; i < numProjectiles; i++) {
        auto projectile = spawner->createProjectile(owner->GetScene(), owner);
        if (!projectile) {
            continue;
        }

        projectile->SetPosition(params.firePosition);

        // Calcula o ângulo e a direção
        float currentAngle = angleStart + (static_cast<float>(i) * angleStep);
        float xCoordRad = Math::Cos(currentAngle * Math::Pi / 180);
        float yCoordRad = Math::Sin(currentAngle * Math::Pi / 180);
        Vector2 directionVector = Vector2(xCoordRad, yCoordRad);
        directionVector.Normalize();

        //  ------ A LÓGICA DE "WAVE" ------ //

        // Começa invisível
        if (auto drawComp = projectile->GetComponent<DrawAnimatedComponent>()) {
            drawComp->SetIsVisible(false);
        }

        // Começa parado
        projectile->GetComponent<RigidBodyComponent>()->SetVelocity(Vector2::Zero);

        // Insere o "despertador" (ActivateBehavior)
        // O projétil 'i' vai acordar após (i * creationSpeed) segundos.
        Vector2 finalVelocity = directionVector * projectileSpeed;
        projectile->insertBehavior<ActivateBehavior>(i * creationSpeed, finalVelocity);

        // Transfere a posse do projétil para o vetor
        projectiles.push_back(std::move(projectile));
    }

    return projectiles;
}