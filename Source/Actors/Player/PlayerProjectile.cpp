//
// Created by nslop on 19/11/2024.
//

#include "Player.h"
#include "../../Game.h"
#include "PlayerProjectile.h"

#include "../../Components/RigidBodyComponent.h"
#include "../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Teachers/Boss.h"
#include "../../Scenes/Battle/Battle.h"



PlayerProjectile::PlayerProjectile(Scene *scene, Player *owner) :
        Projectile(scene, owner)

{

    mOwner = owner;

    std::string spritePath = "../Assets/Player/PlayerProjectile1.png";
    std::string dataPath = "../Assets/Player/PlayerProjectile1.json";

    auto drawComp = AddComponent<DrawAnimatedComponent>(spritePath, dataPath, 50);

    drawComp->AddAnimation("Active", {0,1,2,3,4,5,6});
    drawComp->SetAnimation("Active");

    // TODO 100.0 - Modo dificil o projétil só vai pra cima, Modo normal ela segue o Boss
    auto rigidComp = AddComponent<RigidBodyComponent>();
    drawComp->SetIsVisible(false);

    auto collider = AddComponent<CircleColliderComponent>(static_cast<float>(drawComp->GetSpriteWidth()) / 2.f);
    collider->SetEnabled(false);
    collider->SetTag(ColliderTag::PlayerProjectile);

}

bool PlayerProjectile::IsOffScreen() const
{
    const auto w = static_cast<float>(mScene->GetGame()->GetWindowWidth());
    const auto h = static_cast<float>(mScene->GetGame()->GetWindowHeight());

    if (GetPosition().x > w * 2 || GetPosition().x < -w)
        return true;
    if (GetPosition().y > h * 2 || GetPosition().y < -h)
        return true;

    return false;
}


void PlayerProjectile::OnUpdate(float deltaTime) {
    Projectile::OnUpdate(deltaTime);

    //----- ULTIMA COISA DO UPDATE: Ver colisão. Pois com colisão o objeto pode ser destruído.
}

void PlayerProjectile::ActivateProjectile() {
    SetState(ActorState::Active);

    // Liga os componentes
    if (auto comp = GetComponent<DrawAnimatedComponent>()) {
        comp->SetIsVisible(true);
        comp->SetEnabled(true);
    }
    if (auto comp = GetComponent<CircleColliderComponent>()) {
        comp->SetEnabled(true);
    }

    SetPosition(mOwner->GetPosition());
}

void PlayerProjectile::DeactivateProjectile() {
    SetState(ActorState::Destroy);
}

void PlayerProjectile::OnCollision(Actor *other) {

    if (!other) return;

    auto otherCollider = other->GetComponent<CircleColliderComponent>();
    if (!otherCollider) return;

    if (otherCollider->GetColliderTag() == ColliderTag::Boss) {
        DeactivateProjectile();
    }
}

