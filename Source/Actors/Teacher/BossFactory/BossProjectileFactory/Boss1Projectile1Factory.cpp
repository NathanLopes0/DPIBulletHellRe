//
// Created by nslop on 10/12/2024.
//

#include "Boss1Projectile1Factory.h"
#include "../../Boss.h"
#include "../../Bosses/BossesProjectiles/BossProjectile.h"



Projectile *Boss1Projectile1Factory::createProjectile(Scene* scene, Boss* owner) {


    auto projectile = new BossProjectile(scene, owner);
    auto projectileDrawComponent = new DrawAnimatedComponent(projectile, mSpritePath, mDataPath, 60);
    auto projectileCollider = new CircleColliderComponent(projectile, (float)projectileDrawComponent->GetSpriteWidth() / 2.f);

    configureBaseDrawComponent(projectileDrawComponent);
    projectileDrawComponent->AddAnimation("Homing", {1});
    projectile->insertComponents(projectileDrawComponent, projectileCollider);

    projectile->SetState(ActorState::Active);
    projectile->GetComponent<DrawAnimatedComponent>()->SetIsVisible(true);

    projectile->SetPosition(owner->GetPosition());

    //setar o angulo e a speed no CircleSpreadAttack

    return projectile;

}
