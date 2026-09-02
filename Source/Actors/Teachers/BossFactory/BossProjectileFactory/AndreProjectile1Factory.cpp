//
// Created by gensh on 27/11/2025.
//

#include "AndreProjectile1Factory.h"

#include <vector>
#include <SDL_log.h>

#include "../../Bosses/Andre.h"

#include "../../../../Components/RigidBodyComponent.h"
#include "../../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Bosses/BossesProjectiles/AndreBossProjectile.h"

std::unique_ptr<Projectile> AndreProjectile1Factory::createProjectile(Scene *scene, Actor *owner) {

    auto bossOwner = dynamic_cast<Andre*>(owner);
    if (!bossOwner) {
        SDL_Log("ERRO FATAL: AndreProjectile1Factory recebeu um owner que nao e um Boss!");
        return nullptr;
    }

    // -- CRIAÇÃO DO PROJÉTIL -- //
    auto projectile = std::make_unique<AndreBossProjectile>(scene, bossOwner);
    projectile->SetScale(2.f);

    projectile->SetPosition(bossOwner->GetPosition());
    projectile->AddComponent<RigidBodyComponent>();

    auto drawComp = projectile->AddComponent<DrawAnimatedComponent>(mSpritePath, mDataPath);
    drawComp->AddAnimation("Graph1", {0});
    drawComp->AddAnimation("Graph2", {1});
    drawComp->AddAnimation("Graph3", {2});

    drawComp->SetAnimation("Graph1");
    drawComp->SetIsVisible(true);

    // Adiciona colisor
    float colliderRadius = static_cast<float>(drawComp->GetSpriteWidth()) / 2.f;
    auto collider = projectile->AddComponent<CircleColliderComponent>(colliderRadius);

    projectile->SetState(ActorState::Active);
    return projectile;

}

std::unique_ptr<Projectile> AndreProjectile1Factory::Acquire(Scene* scene, Actor* owner) {

    auto created = mPool.Acquire([this, scene, owner]() {
        return std::unique_ptr<AndreBossProjectile>(
            dynamic_cast<AndreBossProjectile*>(createProjectile(scene, owner).release())
        );
    });

    if (!created) {
        SDL_Log("ERRO FATAL: AndreProjectile1Factory::Acquire falhou em criar/reciclar projetil!");
        return nullptr;
    }

    created->SetOwner(owner);
    created->SetOriginFactory(this);

    return created;
}

void AndreProjectile1Factory::Release(std::unique_ptr<Projectile> projectile) {

    if (!dynamic_cast<AndreBossProjectile*>(projectile.get())) {
        SDL_Log("ERRO: AndreProjectile1Factory::Release recebeu um projetil de tipo incompativel!");
        return;
    }

    mPool.Release(std::unique_ptr<AndreBossProjectile>(
        dynamic_cast<AndreBossProjectile*>(projectile.release())
    ));
}

void AndreProjectile1Factory::Prewarm(Scene* scene, Actor* owner, int count) {

    std::vector<std::unique_ptr<Projectile>> held;
    held.reserve(count);

    for (int i = 0; i < count; ++i) {
        auto p = Acquire(scene, owner);
        if (!p) {
            SDL_Log("AVISO: AndreProjectile1Factory::Prewarm falhou ao criar instancia %d de %d.", i, count);
            continue;
        }
        held.push_back(std::move(p));
    }

    for (auto& p : held) {
        Release(std::move(p));
    }
}
