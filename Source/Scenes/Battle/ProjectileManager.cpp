//
// Created by gensh on 15/10/2025.
//

#include "ProjectileManager.h"
#include "../../Actors/Player/Player.h"
#include "../../Actors/Player/PlayerProjectile.h"
#include "../../Actors/Teachers/Bosses/BossesProjectiles/BossProjectile.h"
#include "../../Actors/ProjectileFactory.h" // Necessário para chamar factory->Release(...)
#include "../../Actors/Actor.h" // Necessário para Actor::State
#include <algorithm>          // Necessário para std::remove_if e std::move
#include <SDL_log.h>

ProjectileManager::ProjectileManager(Scene *owner)
    : mOwnerScene(owner)
{

}

ProjectileManager::~ProjectileManager() = default;

void ProjectileManager::Update(float deltaTime)
{
    // 1.
    for (const auto& proj : mPlayerProjectiles)
    {
        proj->Update(deltaTime);
    }
    for (const auto& proj : mBossProjectiles)
    {
        proj->Update(deltaTime);
    }
    // 2.
    CleanupProjectiles();
}

void ProjectileManager::AddPlayerProjectile(std::unique_ptr<PlayerProjectile> projectile)
{
    mPlayerProjectiles.emplace_back(std::move(projectile));
}

void ProjectileManager::AddBossProjectiles(std::vector<std::unique_ptr<BossProjectile>> projectiles)
{
    mBossProjectiles.insert(mBossProjectiles.end(),
                            std::make_move_iterator(projectiles.begin()),
                            std::make_move_iterator(projectiles.end()));
}

void ProjectileManager::DescartarProjeteisDe(const Actor* dono) const {

    if (!dono) return;

    for (auto& proj : mBossProjectiles) {
        if (!proj || proj->GetOwner() != dono) continue;

        // A ordem importa.
        proj->SetOriginFactory(nullptr);
        proj->Kill();
    }
}

void ProjectileManager::ClearBossProjectiles() const {
    // Marcado como Inactive (não Destroy).
    for (auto& proj : mBossProjectiles) {
        if (!proj) continue;
        const auto estado = proj->GetState();
        if (estado == ActorState::Active || estado == ActorState::Paused) {
            proj->Kill();
        }
    }
}

void ProjectileManager::Cleanup()
{
    CleanupProjectiles();
}

void ProjectileManager::CleanupProjectiles()
{
    // ---- Player: sem pooling (fora do escopo desta mudança) ----
    auto is_dead = [](const auto& proj) { return proj->GetState() == ActorState::Destroy; };
    mPlayerProjectiles.erase(std::remove_if(mPlayerProjectiles.begin(), mPlayerProjectiles.end(), is_dead), mPlayerProjectiles.end());

    // ---- Boss: com pooling ----
    for (auto& proj : mBossProjectiles) {
        if (proj && proj->GetState() == ActorState::Inactive) {
            if (auto* factory = proj->GetOriginFactory()) {
                factory->Release(std::move(proj));
            } else {
                // Sem factory de origem registrada: não há para onde devolver.
                SDL_Log("AVISO: BossProjectile Inactive sem OriginFactory - nao pode ser reciclado, sera destruido.");
                proj.reset();
            }
        }
    }

    auto should_remove = [](const auto& proj) {
        return !proj || proj->GetState() == ActorState::Destroy;
    };
    mBossProjectiles.erase(std::remove_if(mBossProjectiles.begin(), mBossProjectiles.end(), should_remove), mBossProjectiles.end());
}
