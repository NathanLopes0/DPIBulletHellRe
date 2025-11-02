//
// Created by gensh on 18/10/2025.
//

// SallesProjectile1Factory.cpp (VERSÃO REFATORADA E AUTOMATIZADA)

#include "SallesProjectile1Factory.h"
#include "../../Bosses/Salles.h"
#include "../../Bosses/BossesProjectiles/SallesBossProjectile.h"
#include "../../../../Components/RigidBodyComponent.h"
#include "../../../../Components/DrawComponents/DrawSpriteComponent.h"
#include "../../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "SDL_log.h"
#include "../../../../Components/DrawComponents/DrawAnimatedComponent.h"


std::unique_ptr<Projectile> SallesProjectile1Factory::createProjectile(Scene* scene, Actor* owner)
{
    // VERIFICAÇÃO DE SEGURANÇA: O 'owner' é realmente um Boss?
    auto* bossOwner = dynamic_cast<Salles*>(owner);
    if (!bossOwner)
    {
        SDL_Log("ERRO FATAL: SallesProjectile1Factory recebeu um 'owner' que nao e um Boss!");
        return nullptr; // Retorna um ponteiro nulo para sinalizar a falha
    }

    // CRIAÇÃO: Usamos std::make_unique. A posse é nossa (por enquanto).
    auto projectile = std::make_unique<SallesBossProjectile>(scene, bossOwner);

    // Usamos AddComponent<T>
    // O projétil é dono de seus componentes.

    // Posição inicial (antes de adicionar componentes)
    projectile->SetPosition(bossOwner->GetPosition());

    // Adiciona o RigidBody (necessário para velocidade)
    projectile->AddComponent<RigidBodyComponent>();

    // Adiciona o componente de desenho
    auto drawComp = projectile->AddComponent<DrawAnimatedComponent>("../Assets/Teachers/Projectiles/Capivara.png",
                                                                    "../Assets/Teachers/Projectiles/Capivara.json");
    drawComp->AddAnimation("Normal", {0});
    drawComp->SetAnimation("Normal");

    drawComp->SetIsVisible(true);

    // Adiciona o colisor (baseado no tamanho do desenho)
    float colliderRadius = static_cast<float>(drawComp->GetSpriteWidth()) / 2.0f;
    projectile->AddComponent<CircleColliderComponent>(colliderRadius);

    // Ativa o projétil
    projectile->SetState(ActorState::Active);

    // TRANSFERÊNCIA DE POSSE:
    //    Retornamos o unique_ptr. A posse será transferida para quem chamou
    //    (provavelmente a AttackStrategy, que passará para o ProjectileManager em Battle).
    return projectile;
}
