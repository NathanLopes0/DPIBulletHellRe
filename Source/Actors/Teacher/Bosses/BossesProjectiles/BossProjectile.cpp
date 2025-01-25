//
// Created by nslop on 21/11/2024.
//


#include <algorithm>
#include "../../Boss.h"
#include "BossProjectile.h"

BossProjectile::BossProjectile(Scene *scene, Boss *owner)
:Projectile(scene),
mOwner(owner)
{
    mScene->AddToBossProjectiles(this);
}

void BossProjectile::OnUpdate(float deltaTime) {

    //O OnUpdate do Projectile já vê se ele ta dentro dos limites da tela, e se nao tiver, transforma o State em Destroy.
    Projectile::OnUpdate(deltaTime);
    updateBehaviors(deltaTime);

    auto tempVec = Vector2::Normalize(mRigidBodyComponent->GetVelocity());
    //mRigidBodyComponent->SetVelocity(tempVec * mFowardSpeed);
}

bool BossProjectile::InsideProjectileLimit() const {

    auto currPos = GetPosition();
    auto width = mScene->GetGame()->GetWindowWidth();
    auto height = mScene->GetGame()->GetWindowHeight();
    auto spriteWidth = mDrawComponent->GetSpriteWidth();
    auto spriteHeight = mDrawComponent->GetSpriteHeight();

    if(!(currPos.x <= (float)width + (float)spriteWidth && currPos.x >= (float)-spriteWidth - (float)width / 12
        && currPos.y <= (float)height + (float)spriteHeight && currPos.y >= (float)-spriteHeight - (float)height) / 12)
        return false;
    return true;

}

void BossProjectile::insertComponents(DrawAnimatedComponent *pComponent = nullptr,
                                      CircleColliderComponent *pComponent1 = nullptr) {
    mDrawComponent = new DrawAnimatedComponent(*pComponent);
    mColliderComponent = new CircleColliderComponent(*pComponent1);

}

const Vector2& BossProjectile::GetPlayerDirection() {
    auto projPos = GetPosition();
    auto playerPos = GetPlayerPosition();

    auto direction = new Vector2(playerPos - projPos);
    direction->Normalize();

    return *direction;

}

const Vector2 &BossProjectile::GetPlayerPosition() {
    return mScene->GetPlayerPosition();
}

void BossProjectile::updateBehaviors(float deltaTime) {
    for(auto& behavior : mBehaviors) {
        behavior->update(this, deltaTime);
    }

    mBehaviors.erase(std::remove_if(mBehaviors.begin(), mBehaviors.end(),
                                    [](const std::unique_ptr<ProjectileBehavior>& behavior) {
        return behavior->isFinished();
    }), mBehaviors.end());

}

void BossProjectile::Kill() {
    auto bossP = mScene->GetBossProjectiles();
    bossP.erase(std::remove(bossP.begin(), bossP.end(), this), bossP.end());
    mScene->SetBossProjectiles(bossP);
}



