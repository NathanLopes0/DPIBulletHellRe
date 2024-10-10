//
// Created by nslop on 25/09/2024.
//

#include "Salles.h"
#include "BossStates/BossState.h"

Salles::Salles(Scene *scene, const std::string& spritePath, const std::string& dataPath) : Boss(scene) {

    SDL_Log("Constructor - Salles Boss");
    //Draw Component commands
    mDrawComponent = new DrawAnimatedComponent(this, spritePath, dataPath);
    mDrawComponent->AddAnimation("Idle", {0});
    mDrawComponent->SetAnimation("Idle");

    //Rigid Body Component commands
    mRigidBodyComponent = new RigidBodyComponent(this);

    //finite state machine component commands
    mFSMComponent = new FSMComponent(this);

    new BossState(mFSMComponent, "Start");
//    new BossState(mFSMComponent, "Stage 1");
//    new BossState(mFSMComponent, "Stage 2");
//    new BossState(mFSMComponent, "Stage 3");
//    new BossState(mFSMComponent, "End Stage");

    mFSMComponent->Start("Start");
    Start();

}

void Salles::OnUpdate(float deltaTime) {

}



//--------------STATE fUNCTIONS----------------//


void Salles::Movement1() {

}

void Salles::Movement2() {

}

void Salles::Movement3() {

}

void Salles::Attack1() {

}

void Salles::Attack2() {

}

void Salles::Attack3() {

}

void Salles::Start() {
    auto midWidth = mScene->GetGame()->GetWindowWidth() / 2;
    auto spriteHeight = mDrawComponent->GetSpriteHeight();

    SetPosition(Vector2((float)midWidth, (float)-spriteHeight));
    mRigidBodyComponent->SetVelocity(Vector2(0,60));
}


