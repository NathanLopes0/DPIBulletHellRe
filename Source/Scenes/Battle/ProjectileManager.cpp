//
// Created by gensh on 15/10/2025.
//

#include "ProjectileManager.h"
#include "../../Actors/Player/Player.h"
#include "../../Actors/Player/PlayerProjectile.h"
#include "../../Actors/Teachers/Bosses/BossesProjectiles/BossProjectile.h"
#include "../../Components/RigidBodyComponent.h"
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
    // 1. Atualiza todos os projéteis de ambos os lados
    for (const auto& proj : mPlayerProjectiles)
    {
        proj->Update(deltaTime);
    }
    for (const auto& proj : mBossProjectiles)
    {
        proj->Update(deltaTime);
    }
    // 2. Limpa os projéteis que foram marcados como "Destroy"
    CleanupProjectiles();
}

void ProjectileManager::AddPlayerProjectile(std::unique_ptr<PlayerProjectile> projectile)
{
    mPlayerProjectiles.emplace_back(std::move(projectile));
}

void ProjectileManager::AddBossProjectile(std::unique_ptr<BossProjectile> projectile)
{
    mBossProjectiles.emplace_back(std::move(projectile));
}

void ProjectileManager::AddBossProjectiles(std::vector<std::unique_ptr<BossProjectile>> projectiles)
{
    // std::move transfere os elementos do vetor recebido para o nosso vetor de membros,
    // o que é muito mais eficiente do que copiar um por um.
    mBossProjectiles.insert(mBossProjectiles.end(),
                            std::make_move_iterator(projectiles.begin()),
                            std::make_move_iterator(projectiles.end()));
}

void ProjectileManager::ClearBossProjectiles() {
    for (auto& proj : mBossProjectiles) {
        if (proj->GetState() == ActorState::Active) {
            proj->SetState(ActorState::Destroy);
        }
    }
}



void ProjectileManager::CleanupProjectiles()
{
    // Usa o "erase-remove idiom" para limpar os projéteis mortos de forma eficiente
    auto is_dead = [](const auto& proj) { return proj->GetState() == ActorState::Destroy; };

    mPlayerProjectiles.erase(std::remove_if(mPlayerProjectiles.begin(), mPlayerProjectiles.end(), is_dead), mPlayerProjectiles.end());
    mBossProjectiles.erase(std::remove_if(mBossProjectiles.begin(), mBossProjectiles.end(), is_dead), mBossProjectiles.end());
}
