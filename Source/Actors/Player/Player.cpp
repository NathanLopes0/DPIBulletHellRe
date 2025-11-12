//
// Created by nslop on 03/10/2024.
//
#include <vector>
#include "Player.h"
#include "PlayerProjectile.h"
#include "../../Scenes/Battle/Battle.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Scenes/Battle/ProjectileManager.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Components/ColliderComponents/CircleColliderComponent.h"

#define NUM_PROJECTILES_VECTOR 500
#define ATK_TIMER_START_FULL 0.12f
#define PLAYER_SPEED_START 300.0f
#define BASE_SHOOT_FORWARD_SPEED 800.0f

Player::Player(Scene* scene) :
    Actor(scene),
    playerSpeed(PLAYER_SPEED_START),
    atkTimer(ATK_TIMER_START_FULL),
    mMoving(false)

{

    std::string spritePath = "../Assets/Player/DPIBHPlayer.png";
    std::string dataPath = "../Assets/Player/DPIBHPlayer.json";
    auto drawComp = AddComponent<DrawAnimatedComponent>(spritePath, dataPath);
    drawComp->AddAnimation("Moving", {0,1,2,3});
    drawComp->AddAnimation("Idle", {1});
    drawComp->SetAnimation("Idle");

    AddComponent<RigidBodyComponent>();
    auto colliderComp = AddComponent<CircleColliderComponent>(static_cast<float>(drawComp->GetSpriteWidth()) / 2.f);
    colliderComp->SetTag(ColliderTag::Player);

}


void Player::OnProcessInput(const Uint8 *keyState) {

    keyState = SDL_GetKeyboardState(nullptr);

    MoveInput(keyState);
    ShootInput(keyState);
    SpecialInput(keyState);

}

void Player::OnUpdate(float deltaTime) {

    HandleAnimation();
    DecreaseAtkTimer(deltaTime);

}

void Player::MoveInput(const Uint8 *keyState) {

    float newXSpeed = 0;
    float newYSpeed = 0;

    // ------ verificação de movimento CIMA/BAIXO ------
    if(keyState[SDL_SCANCODE_W] || keyState[SDL_SCANCODE_UP])
        newYSpeed -= playerSpeed;
    else if(keyState[SDL_SCANCODE_S] || keyState[SDL_SCANCODE_DOWN])
        newYSpeed += playerSpeed;

    // ------ verificação de movimento ESQUERDA/DIREITA ------
    if(keyState[SDL_SCANCODE_A] || keyState[SDL_SCANCODE_RIGHT])
        newXSpeed -= playerSpeed;
    else if (keyState[SDL_SCANCODE_D] || keyState[SDL_SCANCODE_LEFT])
        newXSpeed += playerSpeed;

    //vendo se está movendo ou não para mudar a variável mMoving (pra mudar a animação de acordo)
    if(newXSpeed == 0 && newYSpeed == 0) mMoving = false;
    else mMoving = true;

    GetComponent<RigidBodyComponent>()->SetVelocity(Vector2(newXSpeed, newYSpeed));
    BorderLimitCheck();

}

void Player::ShootInput(const Uint8 *keyState) {

    if(keyState[SDL_SCANCODE_SPACE]) {
        Shoot();
    }

}

void Player::SpecialInput(const Uint8 *keyState) {

    if(keyState[SDL_SCANCODE_B]) {
        //Bomb();
    }

}

void Player::HandleAnimation() {

    if(mMoving) GetComponent<DrawAnimatedComponent>()->SetAnimation("Moving");
    else GetComponent<DrawAnimatedComponent>()->SetAnimation("Idle");

}

//ShootInput Subfunction
void Player::Shoot() {


    if (atkTimer > 0) return;

    auto battle = dynamic_cast<Battle*>(GetScene());
    if (!battle) return;

    auto projManager = battle->GetProjectileManager();
    if (!projManager) return;

    // TODO - MATANDO o Object Pooling - mudar pra chamada de função em ProjectileManager
    auto projectile = std::make_unique<PlayerProjectile>(GetScene(), this);

    // Posiciona o projétil na frente do jogador
    projectile->SetPosition(GetPosition());

    // Define velocidade inicial (sobe)
    projectile->GetComponent<RigidBodyComponent>()->SetVelocity(Vector2(0.0f, -BASE_SHOOT_FORWARD_SPEED));

    // Ativa e torna visível
    projectile->ActivateProjectile();

    // Entrega ownership ao manager
    projManager->AddPlayerProjectile(std::move(projectile));

    atkTimer = ATK_TIMER_START_FULL;
}

//OnUpdate Subfunction
void Player::DecreaseAtkTimer(float deltaTime) {
    if(atkTimer > 0)
        atkTimer -= deltaTime;
}

// TODO 15.0 - Não deixar o jogador sair das bordas de tela (fazer depois que decidir os limites da tela)
void Player::BorderLimitCheck() {

}
