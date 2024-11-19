//
// Created by nslop on 18/11/2024.
//

#include "Projectile.h"
#include "../../Game.h"
#include "../../Scenes/Scene.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Components/ColliderComponents/CircleColliderComponent.h"


Projectile::Projectile(class Scene *scene) :
        Actor(scene) {

    std::string spritePath = "../Assets/Player/DPIBHReProjectilePrototype.png";
    std::string dataPath = "../Assets/Player/DPIBHReProjectilePrototype.json";

    mDrawComponent = new DrawAnimatedComponent(this, spritePath, dataPath);
    mDrawComponent->AddAnimation("Active", {0,1,2,3,4,5,6,7});
    mDrawComponent->SetAnimation("Active");

    mColliderComponent = new CircleColliderComponent(this, (float)mDrawComponent->GetSpriteWidth() / 2.f);
    mRigidBodyComponent = new RigidBodyComponent(this);

}

void Projectile::OnUpdate(float deltaTime) {
    if(!InsideProjectileLimit()) {
        DeactivateProjectile();
    }

}

void Projectile::OnCollision() {

}

bool Projectile::InsideProjectileLimit() {

    //Os limites são estar a uma tela a mais de distancia do limite da tela(de qualquer lado q seja)

    if(GetPosition().x > (float)mScene->GetGame()->GetWindowWidth() * 2 ||
        GetPosition().x < -1 * (float)mScene->GetGame()->GetWindowWidth())
        return false;

    if(GetPosition().y > (float)mScene->GetGame()->GetWindowHeight() * 2 ||
        GetPosition().y < -1 * (float)mScene->GetGame()->GetWindowHeight())
        return false;

    return true;
}

void Projectile::DeactivateProjectile() {
    SetState(ActorState::Paused);
    mDrawComponent->SetIsVisible(false);
}