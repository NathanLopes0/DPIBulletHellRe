//
// Created by nslop on 03/10/2024.
//
#include <vector>
#include "Player.h"
#include "PlayerProjectile.h"
#include "../../Scenes/Scene.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Components/ColliderComponents/CircleColliderComponent.h"

#define NUM_PROJECTILES_VECTOR 500
#define ATK_TIMER_START_FULL 0.12
#define PLAYER_SPEED_START 300.0f

Player::Player(Scene* scene) :
    Actor(scene),
    playerSpeed(PLAYER_SPEED_START),
    atkTimer(ATK_TIMER_START_FULL)

{

    std::string spritePath = "../Assets/Player/DPIBHPlayer.png";
    std::string dataPath = "../Assets/Player/DPIBHPlayer.json";
    mDrawComponent = new DrawAnimatedComponent(this, spritePath, dataPath);
    mDrawComponent->AddAnimation("Moving", {0, 1, 2, 3});
    mDrawComponent->AddAnimation("Idle", {1});
    mDrawComponent->SetAnimation("Idle"); mMoving = false; //ambos na mesma linha pois andam juntos

    mRigidBodyComponent = new RigidBodyComponent(this);
    mColliderComponent = new CircleColliderComponent(this, (float)mDrawComponent->GetSpriteWidth() / 2.f);

    //aloca um número considerável de Projectiles para que não haja realocação de vetor no meio do jogo.
    for(int i = 0; i < NUM_PROJECTILES_VECTOR; i++) {
        mProjectiles.emplace_back(new PlayerProjectile(mScene, this));
        mProjectiles[i]->DeactivateProjectile();
    }
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

    if(mMoving) mDrawComponent->SetAnimation("Moving");
    else mDrawComponent->SetAnimation("Idle");

}

//ShootInput Subfunction
void Player::Shoot() {
    if(atkTimer <= 0) {
        for(auto it : mProjectiles) {
            if(it->GetState() == ActorState::Paused) {
                it->ActivateProjectile();
                break;
            }
        }

        // TODO 15.0 - INSERIR SOM DE TIRO AQUI

        // TODO 99.0 - Colocar uma variavel base de atkSpeed para poder modificar com powerups
        atkTimer = ATK_TIMER_START_FULL;

    }
}

//OnUpdate Subfunction
void Player::DecreaseAtkTimer(float deltaTime) {
    if(atkTimer > 0)
        atkTimer -= deltaTime;
}

// TODO 5.0 - Não deixar o jogador sair das bordas de tela (fazer depois que decidir os limites da tela)
void Player::BorderLimitCheck() {

}
