//
// Created by gensh on 27/11/2025.
//

#include "AndreProjectile1Factory.h"

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
