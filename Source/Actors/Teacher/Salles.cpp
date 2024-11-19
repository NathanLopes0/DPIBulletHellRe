//
// Created by nslop on 25/09/2024.
//

#include "Salles.h"

Salles::Salles(Scene *scene, const std::string& spritePath, const std::string& dataPath) : Boss(scene) {

    SDL_Log("Constructor - Salles Boss");
    //Draw Component commands

    //Coloquei a declaração de Draw no Boss Específico pois eles podem ter animações diferentes.
    mDrawComponent = new DrawAnimatedComponent(this, spritePath, dataPath);
    mDrawComponent->AddAnimation("Idle", {0});
    mDrawComponent->SetAnimation("Idle");

}

void Salles::OnUpdate(float deltaTime) {

    if(mFSMComponent->GetState()->GetName() == "StartState")
    {
        //SDL_Log("%f, %f", GetPosition().x, GetPosition().y);
        Movement0();
    }

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

    mFSMComponent->Start("StartState");

}

bool Salles::Movement0() {
    return true;
}