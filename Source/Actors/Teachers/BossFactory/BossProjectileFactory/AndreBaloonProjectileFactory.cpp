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

    drawComp->AddAnimation("Normal", {0,1,2,3,4,5});
    drawComp->SetAnimation("Normal");

    drawComp->SetIsVisible(true);

    float colliderRadius = static_cast<float>(drawComp->GetSpriteWidth()) / 2.f;
    projectile->AddComponent<CircleColliderComponent>(colliderRadius);

    projectile->SetState(ActorState::Active);

    return projectile;

}
