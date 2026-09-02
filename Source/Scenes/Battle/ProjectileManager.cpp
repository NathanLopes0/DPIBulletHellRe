//
// Created by gensh on 15/10/2025.
//

#include "ProjectileManager.h"
#include "../../Actors/Player/Player.h"
#include "../../Actors/Player/PlayerProjectile.h"
#include "../../Actors/Teachers/Bosses/BossesProjectiles/BossProjectile.h"
#include "../../Actors/ProjectileFactory.h" // Necessário para chamar factory->Release(...)
#include "../../Components/RigidBodyComponent.h"
#include "../../Actors/Actor.h" // Necessário para Actor::State
#include <algorithm>          // Necessário para std::remove_if e std::move
#include <SDL_log.h>

#include "../../Components/DrawComponents/DrawAnimatedComponent.h"

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

void ProjectileManager::ClearBossProjectiles() const {
    // Marcado como Inactive (não Destroy): esses projéteis costumam ser
    // limpos em transições de fase do Boss, que tende a disparar de novo
    // logo em seguida — faz sentido reciclá-los pelo pool em vez de
    // destruí-los e recriar do zero na próxima vez.
    // Antes isto chamava SetState(Inactive) direto, pulando MarkDead() - e era
    // MarkDead() quem escondia o sprite. Resultado: ao usar um Ponto Extra, os
    // projeteis voltavam ao pool ainda VISIVEIS e ficavam congelados na tela
    // pelo resto da batalha. Kill() delega para MarkDead(), entao limpar a tela
    // agora segue exatamente o mesmo caminho de um projetil que morre saindo da
    // tela ou colidindo.
    for (auto& proj : mBossProjectiles) {
        if (proj && proj->GetState() == ActorState::Active) {
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
    // --- Player: sem pooling (fora do escopo desta mudança). ---
    // PlayerProjectile nunca é marcado Inactive (só Destroy), então o
    // comportamento aqui continua idêntico ao de antes do pooling.
    auto is_dead = [](const auto& proj) { return proj->GetState() == ActorState::Destroy; };
    mPlayerProjectiles.erase(std::remove_if(mPlayerProjectiles.begin(), mPlayerProjectiles.end(), is_dead), mPlayerProjectiles.end());

    // --- Boss: com pooling. ---
    // Precisamos distinguir dois destinos antes de tirar do vetor:
    //  - ActorState::Destroy   -> morre de verdade (erase normal, unique_ptr morre).
    //  - ActorState::Inactive  -> foi devolvido ao pool da sua Factory de
    //    origem (Release), então o vetor deste manager só solta a posse SEM
    //    destruir (o objeto continua vivo, guardado dentro do ProjectilePool).
    //
    // Não dá para usar só std::remove_if aqui: precisamos executar o Release()
    // como um efeito colateral, exatamente uma vez por projétil Inactive,
    // antes de descartar a entrada do vetor. Por isso o loop manual abaixo.
    for (auto& proj : mBossProjectiles) {
        if (proj && proj->GetState() == ActorState::Inactive) {
            if (auto* factory = proj->GetOriginFactory()) {
                factory->Release(std::move(proj));
            } else {
                // Sem factory de origem registrada: não há para onde devolver.
                // Isso não deveria acontecer para um BossProjectile (todos
                // passam por ProjectileFactory::Acquire, que sempre seta
                // isso) — loga para investigação, mas ainda assim descarta
                // com segurança (proj vira nullptr, será removido abaixo).
                SDL_Log("AVISO: BossProjectile Inactive sem OriginFactory - nao pode ser reciclado, sera destruido.");
                proj.reset();
            }
        }
    }

    // Agora remove do vetor deste manager: os que morreram de verdade
    // (Destroy) E os que já foram movidos para dentro de algum pool ou
    // resetados acima (proj == nullptr após Release()/reset(), já que
    // std::move deixa o unique_ptr de origem vazio).
    auto should_remove = [](const auto& proj) {
        return !proj || proj->GetState() == ActorState::Destroy;
    };
    mBossProjectiles.erase(std::remove_if(mBossProjectiles.begin(), mBossProjectiles.end(), should_remove), mBossProjectiles.end());
}
