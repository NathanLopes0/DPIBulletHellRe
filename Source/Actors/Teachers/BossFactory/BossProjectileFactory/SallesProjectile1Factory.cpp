//
// Created by gensh on 18/10/2025.
//

// SallesProjectile1Factory.cpp (VERSÃO REFATORADA E AUTOMATIZADA)

#include "SallesProjectile1Factory.h"
#include <vector>
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

    // CRIAÇÃO: usar std::make_unique. A posse é nossa (por enquanto).
    auto projectile = std::make_unique<SallesBossProjectile>(scene, bossOwner);

    // Usamos AddComponent<T>
    // O projétil é dono de seus componentes.

    // Posição inicial (antes de adicionar componentes)
    projectile->SetPosition(bossOwner->GetPosition());

    // Adiciona o RigidBody (necessário para velocidade)
    projectile->AddComponent<RigidBodyComponent>();

    // Adiciona o componente de desenho
    auto drawComp = projectile->AddComponent<DrawAnimatedComponent>("../Assets/Teachers/Projectiles/DPIBHSallesCapivara.png",
                                                                    "../Assets/Teachers/Projectiles/DPIBHSallesCapivara.json", 90);
    drawComp->AddAnimation("Normal", {0});
    drawComp->AddAnimation("Homing", {1});
    drawComp->SetAnimation("Normal");

    drawComp->SetIsVisible(true);

    // Adiciona o colisor (baseado no tamanho do desenho)
    float colliderRadius = static_cast<float>(drawComp->GetSpriteWidth()) / 2.0f;
    auto collider = projectile->AddComponent<CircleColliderComponent>(colliderRadius);
    collider->SetTag(ColliderTag::BossProjectile);

    // Ativa o projétil
    projectile->SetState(ActorState::Active);

    // TRANSFERÊNCIA DE POSSE:
    //    Retornar unique_ptr. A posse será transferida para quem chamou (tenho que lembrar de transferir la, mas é comportamento padrão)
    //    (quem chamou provavelmente foi a AttackStrategy, que passará para o ProjectileManager em Battle).
    return projectile;
}

std::unique_ptr<Projectile> SallesProjectile1Factory::Acquire(Scene* scene, Actor* owner) {

    auto created = mPool.Acquire([this, scene, owner]() {
        return std::unique_ptr<SallesBossProjectile>(
            dynamic_cast<SallesBossProjectile*>(createProjectile(scene, owner).release())
        );
    });

    if (!created) {
        SDL_Log("ERRO FATAL: SallesProjectile1Factory::Acquire falhou em criar/reciclar projetil!");
        return nullptr;
    }

    created->SetOwner(owner);
    created->SetOriginFactory(this);

    return created;
}

void SallesProjectile1Factory::Release(std::unique_ptr<Projectile> projectile) {

    if (!dynamic_cast<SallesBossProjectile*>(projectile.get())) {
        SDL_Log("ERRO: SallesProjectile1Factory::Release recebeu um projetil de tipo incompativel!");
        return;
    }

    mPool.Release(std::unique_ptr<SallesBossProjectile>(
        dynamic_cast<SallesBossProjectile*>(projectile.release())
    ));
}

void SallesProjectile1Factory::Prewarm(Scene* scene, Actor* owner, int count) {

    std::vector<std::unique_ptr<Projectile>> held;
    held.reserve(count);

    for (int i = 0; i < count; ++i) {
        auto p = Acquire(scene, owner);
        if (!p) {
            SDL_Log("AVISO: SallesProjectile1Factory::Prewarm falhou ao criar instancia %d de %d.", i, count);
            continue;
        }
        held.push_back(std::move(p));
    }

    for (auto& p : held) {
        Release(std::move(p));
    }
}
