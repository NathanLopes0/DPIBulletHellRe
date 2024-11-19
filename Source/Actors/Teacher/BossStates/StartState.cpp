//
// Created by nslop on 15/10/2024.
//

#include "StartState.h"
#include "../Boss.h"

StartState::StartState(FSMComponent *stateMachine) : BossState(stateMachine, "StartState") {

}

void StartState::Start() {

}

void StartState::HandleStateTransition(float stateTime) {
    auto bossPosition = mBoss->GetPosition();
    int nextStateLine = mBoss->GetWindowsHeight() / 8;

    // TODO 2.0 - fazer classe StateOne e pensar em como chamar Movement1() e Attack1().
    // TODO 2.0.1 - talvez chamar Movement1() e Attack1() do StateOne, já que cada boss vai ter Movement e Attack implementado separadamente.
    // TODO 2.0.1 - e também pensar em um jeito de fazer o State sem chamar M1 e A1 dele, e sim o boss perceber os estados e mudar de acordo.

    if(abs((int)bossPosition.y - nextStateLine) < 5) {
        //SDL_Log("----Ir para StateOne----");
        mBoss->GetComponent<RigidBodyComponent>()->SetVelocity(Vector2::Zero);
        //mBoss->GetComponent<FSMComponent>()->SetState("StateOne");
    }
}
