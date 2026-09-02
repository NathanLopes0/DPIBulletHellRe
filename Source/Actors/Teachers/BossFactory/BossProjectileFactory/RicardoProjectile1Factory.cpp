//
// Created by gensh on 26/11/2025.
//

#include "RicardoProjectile1Factory.h"

#include <vector>
#include <complex>

#include "../../Bosses/Ricardo.h"

#include <SDL_log.h>

#include "../../../../Components/RigidBodyComponent.h"
#include "../../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Bosses/BossesProjectiles/RicardoBossProjectile.h"

std::unique_ptr<Projectile> RicardoProjectile1Factory::createProjectile(Scene *scene, Actor *owner) {

    auto* bossOwner = dynamic_cast<Ricardo*>(owner);
    if (!bossOwner) {
        SDL_Log("ERRO FATAL: RicardoProjectile1Factory recebeu um 'owner' que nao e um Boss!");
        return nullptr;
    }

    // -- CRIAÇÃO DO PROJÉTIL -- //
    auto projectile = std::make_unique<RicardoBossProjectile>(scene, bossOwner);

    projectile->SetPosition(bossOwner->GetPosition());

    projectile->AddComponent<RigidBodyComponent>();

    // A Sprite é 16x16, e acho que fica melhor 32x32 então aumentei o Actor
    projectile->SetScale(2.f);
    auto drawComp = projectile->AddComponent<DrawAnimatedComponent>("../Assets/Teachers/Projectiles/DPIBHRicardoProjectile.png",
        mDataPath, 90);

    drawComp->AddAnimation("Normal", {0});
    drawComp->SetAnimation("Normal");

    drawComp->SetIsVisible(true);

    // Adiciona colisor
    float colliderRadius = static_cast<float>(drawComp->GetSpriteWidth()) / 2.f;
    auto collider = projectile->AddComponent<CircleColliderComponent>(colliderRadius);

    projectile->SetState(ActorState::Active);

    return projectile;

}

std::unique_ptr<Projectile> RicardoProjectile1Factory::Acquire(Scene* scene, Actor* owner) {

    auto created = mPool.Acquire([this, scene, owner]() {
        return std::unique_ptr<RicardoBossProjectile>(
            dynamic_cast<RicardoBossProjectile*>(createProjectile(scene, owner).release())
        );
    });

    if (!created) {
        SDL_Log("ERRO FATAL: RicardoProjectile1Factory::Acquire falhou em criar/reciclar projetil!");
        return nullptr;
    }

    created->SetOwner(owner);
    created->SetOriginFactory(this);

    return created;
}

void RicardoProjectile1Factory::Release(std::unique_ptr<Projectile> projectile) {

    if (!dynamic_cast<RicardoBossProjectile*>(projectile.get())) {
        SDL_Log("ERRO: RicardoProjectile1Factory::Release recebeu um projetil de tipo incompativel!");
        return;
    }

    mPool.Release(std::unique_ptr<RicardoBossProjectile>(
        dynamic_cast<RicardoBossProjectile*>(projectile.release())
    ));
}

void RicardoProjectile1Factory::Prewarm(Scene* scene, Actor* owner, int count) {

    std::vector<std::unique_ptr<Projectile>> held;
    held.reserve(count);

    for (int i = 0; i < count; ++i) {
        auto p = Acquire(scene, owner);
        if (!p) {
            SDL_Log("AVISO: RicardoProjectile1Factory::Prewarm falhou ao criar instancia %d de %d.", i, count);
            continue;
        }
        held.push_back(std::move(p));
    }

    for (auto& p : held) {
        Release(std::move(p));
    }
}
