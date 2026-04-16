//
// Created by gensh on 24/03/2026.
//

#include "AndreBaloonProjectileFactory.h"

#include <SDL_log.h>
#include "../../Bosses/Andre.h"
#include "../../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../../../Components/RigidBodyComponent.h"
#include "../../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Bosses/BossesProjectiles/AndreBaloonProjectile.h"

std::unique_ptr<Projectile> AndreBaloonProjectileFactory::createProjectile(Scene *scene, Actor *owner) {

    auto bossOwner = dynamic_cast<Andre*>(owner);
    if (!bossOwner) {
        SDL_Log("ERRO FATAL: AndreBaloonProjectileFactory recebeu um 'owner' que nao e um Boss!");
        return nullptr;
    }

    // -- CRIAÇÃO DO PROJÉTIL -- //

    auto projectile = std::make_unique<AndreBaloonProjectile>(scene, bossOwner);

    projectile->AddComponent<RigidBodyComponent>();

    projectile->SetScale(2.f);

    auto drawComp = projectile->AddComponent<DrawAnimatedComponent>("../Assets/Teachers/Projectiles/DPIBHAndreBaloon.png",
                                                "../Assets/Teachers/Projectiles/DPIBHAndreBaloon.json", 90);

    drawComp->AddAnimation("Red", {6, 7, 8, 9, 10, 11});
    drawComp->AddAnimation("Blue", {12,13,14,15,16,17});
    drawComp->AddAnimation("Yellow", {18,19,20,21,22,23});

    drawComp->SetIsVisible(true);

    float colliderRadius = static_cast<float>(drawComp->GetSpriteWidth()) / 4.f;
    projectile->AddComponent<CircleColliderComponent>(colliderRadius);

    projectile->SetState(ActorState::Active);

    return projectile;

}
