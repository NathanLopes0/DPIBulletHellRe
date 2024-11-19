//
// Created by nslop on 15/10/2024.
//

#include "StartState.h"
#include "../Boss.h"

StartState::StartState(FSMComponent *stateMachine) : BossState(stateMachine, "StartState") {

}

void StartState::Start() {

    SDL_Log("Starting 'StartState'");

    mBoss->SetState(ActorState::Active);
    mBoss->GetComponent<DrawAnimatedComponent>()->SetIsVisible(true);

    auto midWidth = mBoss->GetWindowsWidth() / 2;
    auto spriteHeight = mBoss->GetSpriteHeight();

    mBoss->SetPosition(Vector2((float)midWidth, -2.f * (float)spriteHeight));
    mBoss->GetComponent<RigidBodyComponent>()->SetVelocity(Vector2(0, 50));
}

void StartState::HandleStateTransition(float stateTime) {
    auto bossPosition = mBoss->GetPosition();
    int nextStateLine = mBoss->GetWindowsHeight() / 8;
    if(abs((int)bossPosition.y - nextStateLine) < 5) {
        SDL_Log("----Ir para StateOne----");
        mBoss->GetComponent<RigidBodyComponent>()->SetVelocity(Vector2::Zero);
        //mBoss->GetComponent<FSMComponent>()->SetState("StateOne");
    }
}
