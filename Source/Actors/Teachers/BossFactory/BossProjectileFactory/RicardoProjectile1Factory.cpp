//
// Created by gensh on 26/11/2025.
//

#include "RicardoProjectile1Factory.h"

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
