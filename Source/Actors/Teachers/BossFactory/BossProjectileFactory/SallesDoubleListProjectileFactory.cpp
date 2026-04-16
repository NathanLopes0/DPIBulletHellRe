//
// Created by gensh on 09/04/2026.
//

#include "SallesDoubleListProjectileFactory.h"

#include <complex>

#include "../../Bosses/Salles.h"
#include "../../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../../../Components/RigidBodyComponent.h"
#include "../../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Bosses/BossesProjectiles/SallesDoubleListProjectile.h"
#include <SDL_log.h>

std::unique_ptr<Projectile> SallesDoubleListProjectileFactory::createProjectile(Scene* scene, Actor* owner) {

    auto BossOwner = dynamic_cast<Salles*>(owner);
    if (!BossOwner) {
        SDL_Log("ERRO FATAL: SallesDoubleListProjectileFactory recebeu um owner que não é um Salles");
        return nullptr;
    }



    // -- CRIAÇÃO DO PROJÉTIL -- //
    //SDL_Log("Criando Projetil DoubleList");
    auto projectile = std::make_unique<SallesDoubleListProjectile>(scene,BossOwner);
    projectile->SetScale(.75f);

    projectile->AddComponent<RigidBodyComponent>();

    //SDL_Log("Criando drawComponent");
    auto drawComp = projectile->AddComponent<DrawAnimatedComponent>("../Assets/Teachers/Projectiles/DPIBHSallesDuplamente.png",
                                        "../Assets/Teachers/Projectiles/DPIBHSallesDuplamente.json", 90);


    drawComp->AddAnimation("Normal", {0,1,2,3,4,5,6,7,8});
    drawComp->SetAnimation("Normal");

    drawComp->SetIsVisible(true);

    float colliderRadius = static_cast<float>(drawComp->GetSpriteHeight()) / 2.f;
    projectile->AddComponent<CircleColliderComponent>(colliderRadius);

    projectile->SetState(ActorState::Active);

    //SDL_Log("Retornando o projetil");
    return projectile;



}
