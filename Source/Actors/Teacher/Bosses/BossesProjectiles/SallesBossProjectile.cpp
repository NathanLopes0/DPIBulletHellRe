//
// Created by nslop on 21/11/2024.
//

#include "../Salles.h"
#include "SallesBossProjectile.h"

SallesBossProjectile::SallesBossProjectile(Scene *scene, Salles *owner, const std::string &spritePath,
                                           const std::string &dataPath) : BossProjectile(scene, owner) {

    mDrawComponent = new DrawAnimatedComponent(this, spritePath, dataPath);
    mColliderComponent = new CircleColliderComponent(this, (float)mDrawComponent->GetSpriteWidth() / 2.f);
    mRigidBodyComponent = new RigidBodyComponent(this);

    mDrawComponent->SetIsVisible(true);
    mDrawComponent->AddAnimation("NoAnim", {0});
    mDrawComponent->SetAnimation("NoAnim");

}
