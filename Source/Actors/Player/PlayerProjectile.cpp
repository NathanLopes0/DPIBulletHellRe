//
// Created by nslop on 19/11/2024.
//

#include "Player.h"
#include "../../Game.h"
#include "PlayerProjectile.h"

#include "../../Components/RigidBodyComponent.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Teacher/Boss.h"
#include "../../Scenes/Battle/Battle.h"



PlayerProjectile::PlayerProjectile(Scene *scene, Player *owner) :
        Projectile(scene, owner)

{

    mOwner = owner;

    std::string spritePath = "../Assets/Player/SimpleProjectile alpha0.png";
    std::string dataPath = "../Assets/Player/SimpleProjectile alpha0.json";

    auto drawComp = AddComponent<DrawAnimatedComponent>(spritePath, dataPath, 50);

    drawComp->AddAnimation("Active", {0,1,2,3,4,5,6});
    drawComp->SetAnimation("Active");


    // TODO 100.0 - Modo dificil o projétil só vai pra cima, Modo normal ela segue o Boss
    auto rigidComp = AddComponent<RigidBodyComponent>();
    drawComp->SetIsVisible(false);

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
    GetComponent<DrawAnimatedComponent>()->SetIsVisible(true);
    SetPosition(mOwner->GetPosition());

    SDL_Log("PlayerProjectile - ativado na pos %.1f, %.1f", GetPosition().x, GetPosition().y);
}

void PlayerProjectile::DeactivateProjectile() {
    SetState(ActorState::Paused);
    GetComponent<DrawAnimatedComponent>()->SetIsVisible(false);
    GetComponent<RigidBodyComponent>()->SetVelocity(Vector2::Zero);
}

bool PlayerProjectile::CheckCollision(Actor *other) {
    if (auto boss = dynamic_cast<Boss*>(other)) {
        DeactivateProjectile();
        return true;
    }
    return false;
}

