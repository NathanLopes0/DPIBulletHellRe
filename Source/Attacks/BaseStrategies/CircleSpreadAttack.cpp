//
// Created by nslop on 26/11/2024.
//

#include <memory>
#include "CircleSpreadAttack.h"

#include <SDL_log.h>

#include "../../Actors/Projectile.h"
#include "../../Actors/Actor.h"
#include "../../Actors/ProjectileFactory.h"
#include "../../Components/RigidBodyComponent.h"
#include "../AttackParameters/AttackParams.h"

CircleSpreadAttack::CircleSpreadAttack(ProjectileFactory* spawner, Actor* owner)
    : IAttackStrategy(spawner, owner)
{

}

std::vector<std::unique_ptr<Projectile>> CircleSpreadAttack::Execute(const AttackParams& params) {

    // Retorno de unique_ptr.
    std::vector<std::unique_ptr<Projectile>> projectiles;

    // Lógica de gameplay lida de params.
    const int numProjectiles = params.numProjectiles;
    const float projectileSpeed = params.projectileSpeed;

    // Evita divisão por zero se numProjectiles for 0.
    if (numProjectiles == 0) {
        return projectiles;
    }

    // Reserva espaço no vetor para eficiência.
    projectiles.reserve(numProjectiles);

    // Lógica matemática.
    const float angleStep = 360.0f / static_cast<float>(numProjectiles);

    for(int i = 0; i < numProjectiles; i++) {

        auto projectile = Acquire();

        // Failsafe: Se a fábrica falhar, pula este projétil.
        if (!projectile) {
            SDL_Log("Failed to create projectile");
            continue;
        }

        // Posicionamento sempre explicito.
        projectile->SetPosition(params.firePosition);

        // Lógica de direção de cada projétil.
        // O anel comeca em zero grau, girado pelo sorteio do disparo (zero sem "variar").
        const float graus = params.giroDoDisparo + static_cast<float>(i) * angleStep;
        const float xCoordRad = Math::Cos(graus * Math::Pi / 180);
        const float yCoordRad = Math::Sin(graus * Math::Pi / 180);
        auto directionVector = Vector2(xCoordRad, yCoordRad);
        directionVector.Normalize();

        // Xc = cos(angle * pi / 180) arccos(xc) = angle * pi / 180.

        // Velocidade usa o projectileSpeed do params, lida lá em cima da função.
        auto rb = projectile->GetComponent<RigidBodyComponent>();
        if (!rb) {
            SDL_Log("Erro em CircleSpreadAttack: projetil sem RigidBodyComponent, pulando.");
            continue;
        }
        rb->SetVelocity(directionVector * projectileSpeed);

        projectiles.push_back(std::move(projectile));
    }

    return projectiles; // Retorna o vetor cheio de projéteis E dono de todos
}

