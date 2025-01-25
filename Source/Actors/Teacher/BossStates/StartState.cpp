//
// Created by nslop on 15/10/2024.
//

#include "StartState.h"
#include "../Boss.h"

StartState::StartState(FSMComponent *stateMachine) : BossState(stateMachine, "StartState") {

}

void StartState::Start() {

}

void StartState::Update(float deltaTime) {

}

void StartState::HandleStateTransition(float stateTime) {

    //Talvez nem todos os bosses terão a mesma entrada, mas vamos ver. Por simplicidade, esse pode ser um padrão.
    auto bossPosition = mBoss->GetPosition();
    int nextStateLine = mBoss->GetWindowsHeight() / 8;

    if(abs((int)bossPosition.y - nextStateLine) < 5) {
        mBoss->GetComponent<RigidBodyComponent>()->SetVelocity(Vector2::Zero);
        mBoss->GetComponent<FSMComponent>()->SetState("StateOne");
    }
}
