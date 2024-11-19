//
// Created by nslop on 03/10/2024.
//

#include "Player.h"
#include "../../Scenes/Scene.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Components/ColliderComponents/CircleColliderComponent.h"


Player::Player(Scene* scene) :
    Actor(scene),
    playerSpeed(300.0f)

{

    std::string spritePath = "../Assets/Player/DPIBHPlayer.png";
    std::string dataPath = "../Assets/Player/DPIBHPlayer.json";
    mDrawComponent = new DrawAnimatedComponent(this, spritePath, dataPath);
    mDrawComponent->AddAnimation("Moving", {0, 1, 2, 3});
    mDrawComponent->AddAnimation("Idle", {1});
    mDrawComponent->SetAnimation("Idle"); mMoving = false; //ambos na mesma linha pois andam juntos

    mRigidBodyComponent = new RigidBodyComponent(this);
    mColliderComponent = new CircleColliderComponent(this, (float)mDrawComponent->GetSpriteWidth() / 2.f);

}


void Player::OnProcessInput(const Uint8 *keyState) {

    keyState = SDL_GetKeyboardState(nullptr);

    MoveInput(keyState);
    ShootInput(keyState);
    SpecialInput(keyState);

}

void Player::OnUpdate(float deltaTime) {

    HandleAnimation();

}

void Player::MoveInput(const Uint8 *keyState) {

    float newXSpeed = 0;
    float newYSpeed = 0;

    if(keyState[SDL_SCANCODE_W] || keyState[SDL_SCANCODE_UP])
        newYSpeed -= playerSpeed;
    else if(keyState[SDL_SCANCODE_S] || keyState[SDL_SCANCODE_DOWN])
        newYSpeed += playerSpeed;

    if(keyState[SDL_SCANCODE_A] || keyState[SDL_SCANCODE_RIGHT])
        newXSpeed -= playerSpeed;
    else if (keyState[SDL_SCANCODE_D] || keyState[SDL_SCANCODE_LEFT])
        newXSpeed += playerSpeed;

    if(newXSpeed == 0 && newYSpeed == 0) mMoving = false;
    else mMoving = true;

    GetComponent<RigidBodyComponent>()->SetVelocity(Vector2(newXSpeed, newYSpeed));

}

void Player::ShootInput(const Uint8 *keyState) {

    if(keyState[SDL_SCANCODE_SPACE]) {
        //Shoot();
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
