//
// Created by nslop on 12/12/2024.
//

// TODO 25.0 - Tirar o include <SDL_log.h>, estou usando só pra debugar o código
#include <SDL_log.h>
#include "StateTwo.h"


StateTwo::StateTwo(FSMComponent *stateMachine) : BossState(stateMachine, "StateTwo") {

}

void StateTwo::Start() {
    SDL_Log("Starting StateTwo");
    SetMaxStateTime(17.f);

}

void StateTwo::Update(float deltaTime) {

}

void StateTwo::HandleStateTransition(float stateTime) {

    if(GetStateTime() > GetMaxStateTime())
    {
        SDL_Log("Fim StateTwo");
    }

}