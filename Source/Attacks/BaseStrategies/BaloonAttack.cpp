//
// Created by gensh on 20/03/2026.
//

#include "BaloonAttack.h"

#include <SDL_log.h>

#include "../../Actors/Projectile.h"
#include "../AttackParameters/AttackParams.h"
#include "../AttackParameters/BaloonAttackParams.h"
#include "../../Actors/ProjectileFactory.h"
#include "../../Actors/Player/Player.h"
#include "../../Scenes/Battle/Battle.h"


BaloonAttack::BaloonAttack(ProjectileFactory* projectileFactory, Actor* owner)
    : mSpawner(projectileFactory),
      mOwner(owner)
{

}

std::vector<std::unique_ptr<Projectile>> BaloonAttack::Execute(const AttackParams& params) {

    // 1. Verifica se os parâmetros são do tipo correto
    const auto baloonParams = dynamic_cast<const BaloonAttackParams*>(&params);
    if (!baloonParams) {
        SDL_Log("BaloonAttack::Execute - Os parametros recebidos não são do tipo BaloonAttackParams");
        return {};
    }

    // 2. Validação semântica dos parâmetros
    if (!ValidateParams(*baloonParams)) {
        SDL_Log("BaloonAttack::Execute - Invalid BaloonAttackParams");
        return {};
    }

    std::vector<std::unique_ptr<Projectile>> projectiles;

    const int numProjectiles = params.numProjectiles;

    if (numProjectiles <= 0) {
        return projectiles;
    }

    // 3. Reserva para eficiência
    projectiles.reserve(numProjectiles);

    // Cena atual
    auto* scene = mOwner->GetScene();
    auto* battle = dynamic_cast<Battle*>(scene);

    if (!battle) {
        SDL_Log("BaloonAttack::Execute - Scene is not Battle");
        return projectiles;
    }

    // 4. MODO 1: spawn aleatório (com ou sem centro no player)
    if (baloonParams->randomSpawn) {

        for (int i = 0; i < numProjectiles; ++i) {

            auto projectile = mSpawner->createProjectile(scene, mOwner);
            if (!projectile) {
                SDL_Log("BaloonAttack::Execute - Failed to create projectile");
                continue;
            }

            SpawnBalloonsAtRandom(*baloonParams, battle, projectile.get());
            projectiles.push_back(std::move(projectile));
        }

        return projectiles;
    }

    // MODO 2: spawn em posições exatas
    const auto& spawnPoints = baloonParams->spawnPoints;
    const int maxSpawns = std::min(
        static_cast<int>(spawnPoints.size()),
        numProjectiles
    );

    for (int i = 0; i < maxSpawns; ++i) {

        auto projectile = mSpawner->createProjectile(scene, mOwner);
        if (!projectile) continue;

        SpawnBalloonAtPosition(
            *baloonParams,
            projectile.get(),
            spawnPoints[i]
        );

        projectiles.push_back(std::move(projectile));
    }

    return projectiles;
}

bool BaloonAttack::ValidateParams(const BaloonAttackParams& params) {
    if (!params.randomSpawn && params.centerOnPlayer) {
        SDL_Log("Invalid BaloonAttackParams: centerOnPlayer cannot be true when randomSpawn is false");
        return false;
    }

    if (!params.randomSpawn && params.spawnPoints.empty()) {
        SDL_Log("Invalid BaloonAttackParams: spawnPoints is empty");
        return false;
    }

    if (params.side == BaloonAttackParams::None) {
        SDL_Log("Invalid BaloonAttackParams: side is None");
        return false;
    }

    return true;
}

Vector2 BaloonAttack::GetDirectionFromSide(const enum BaloonAttackParams::side side) {
    switch (side) {
        case BaloonAttackParams::Down:  return Vector2(0, -1);
        case BaloonAttackParams::Up:    return Vector2(0, 1);
        case BaloonAttackParams::Left:  return Vector2(1, 0);
        case BaloonAttackParams::Right: return Vector2(-1, 0);
        default: return Vector2::Zero;
    }
}

Vector2 BaloonAttack::CalculateSpawnPosition( const BaloonAttackParams& params, const Battle* battle) {
    const auto game = battle->GetGame();
    const Vector2 screenSize(
        game->GetWindowWidth(),
        game->GetWindowHeight()
    );

    Vector2 pos;

    if (params.randomSpawn) {
        // eixo variável
        float variableCoord;

        if (params.centerOnPlayer) {
            const Vector2 playerPos = battle->GetPlayer()->GetPosition();
            variableCoord = Random::GetFloatRange(
                -params.centerOnPlayerOffset,
                params.centerOnPlayerOffset
            );

            variableCoord += (params.side == BaloonAttackParams::Down || params.side == BaloonAttackParams::Up)
                ? playerPos.x
                : playerPos.y;
        } else {
            variableCoord = Random::GetFloatRange(
                0,
                (params.side == BaloonAttackParams::Down || params.side == BaloonAttackParams::Up)
                    ? screenSize.x
                    : screenSize.y
            );
        }

        // eixo fixo
        if (params.side == BaloonAttackParams::Down) {
            pos = Vector2(variableCoord, screenSize.y + 50);
        } else if (params.side == BaloonAttackParams::Up) {
            pos = Vector2(variableCoord, -50);
        } else if (params.side == BaloonAttackParams::Left) {
            pos = Vector2(-50, variableCoord);
        } else if (params.side == BaloonAttackParams::Right) {
            pos = Vector2(screenSize.x + 50, variableCoord);
        }
    }

    return pos;
}

void BaloonAttack::SpawnBalloonsAtRandom(const BaloonAttackParams& params, Battle* battle, Projectile* projectile) {
    Vector2 spawnPos = CalculateSpawnPosition(params, battle);
    projectile->SetPosition(spawnPos);

    Vector2 dir = GetDirectionFromSide(params.side);
    projectile->insertBehavior<ActivateBehavior>(
        Random::GetFloatRange(0, 2),
        dir * params.projectileSpeed
    );
}

void BaloonAttack::SpawnBalloonAtPosition( const BaloonAttackParams& params, Projectile* projectile, const Vector2& position) {
    projectile->SetPosition(position);

    Vector2 dir = GetDirectionFromSide(params.side);
    projectile->insertBehavior<ActivateBehavior>(
        Random::GetFloatRange(0.0f, 3.0f),
        dir * params.projectileSpeed
    );
}