//
// Created by nslop on 19/11/2024.
//

#include "Player.h"
#include "../../Game.h"
#include "PlayerProjectile.h"
#include "../Teacher/Boss.h"

#define BASE_FOWARD_SPEED 800.0


PlayerProjectile::PlayerProjectile(Scene *scene, Player *owner) :
        Projectile(scene)

{

    mOwner = owner;

    std::string spritePath = "../Assets/Player/SimpleProjectile alpha0.png";
    std::string dataPath = "../Assets/Player/SimpleProjectile alpha0.json";

    mDrawComponent = new DrawAnimatedComponent(this, spritePath, dataPath, 50);
    mDrawComponent->AddAnimation("Active", {0,1,2,3,4,5,6});
    mDrawComponent->SetAnimation("Active");


    // TODO 99.0 - Colocar uma variável para ser a mFowardSpeed para que possa mudar com powerups
    mFowardSpeed = BASE_FOWARD_SPEED;

    // TODO 100.0 - Modo dificil o projétil só vai pra cima, Modo normal ela segue o Boss
    mRigidBodyComponent->SetVelocity(Vector2(0, -mFowardSpeed));
    // Será que é bom colocar a velocidade aqui? As vezes é melhor o Player ter mais controle sobre a velocidade desejada
    // do Player Projectile? Que aí ele pode ter ataques diferentes. Porém, ele pode mudar a velocidade dentro de alguma função
    //lá ainda, e essa pode ser a base para que ele não precise colocá-la.

    //pensando... como colocar ela pra seguir o boss no construtor do PlayerProjectile? talvez mais um ponto pra
    //colocar a SetVelocity no Player.

}

bool PlayerProjectile::InsideProjectileLimit() const {

    //Os limites dos projéteis do jogador é estar a uma tela a mais
                //de distância do limite da tela(de qualquer lado q seja)

    if(GetPosition().x > (float)mScene->GetGame()->GetWindowWidth() * 2 ||
       GetPosition().x < -1 * (float)mScene->GetGame()->GetWindowWidth()) {
        return false;
    }

    if(GetPosition().y > (float)mScene->GetGame()->GetWindowHeight() * 2 ||
       GetPosition().y < -1 * (float)mScene->GetGame()->GetWindowHeight()) {
        return false;
    }

    return true;
}

void PlayerProjectile::OnUpdate(float deltaTime) {
    Projectile::OnUpdate(deltaTime);

    //----- ULTIMA COISA DO UPDATE: Ver colisão. Pois com colisão o objeto pode ser destruído.
    if(CheckCollision())
        OnCollision();
}

void PlayerProjectile::ActivateProjectile() {
    SetState(ActorState::Active);
    mDrawComponent->SetIsVisible(true);
    SetPosition(mOwner->GetPosition());
}

void PlayerProjectile::DeactivateProjectile() {
    SetState(ActorState::Paused);
    mDrawComponent->SetIsVisible(false);
}

const CircleColliderComponent* PlayerProjectile::BossCollider() {

    return mScene->GetBoss()->GetComponent<CircleColliderComponent>();

}

bool PlayerProjectile::CheckCollision() {
    if((mScene->GetBoss()->GetPosition() - GetPosition()).Length() < 100)
        return Collided();
    else
        return false;
}


bool PlayerProjectile::Collided() {
    return mColliderComponent->Intersect(*BossCollider());
}

void PlayerProjectile::OnCollision() {

    // TODO 4.1 - antes de desativar o projétil, mudar a nota de acordo com o número de pontos dados por acerto.
    DeactivateProjectile();
}
