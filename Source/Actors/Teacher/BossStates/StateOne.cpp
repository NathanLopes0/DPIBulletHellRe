//
// Created by nslop on 19/11/2024.
//

// TODO 20.0 - Tirar o include <SDL_log.h> daqui, estou usando só pra debugar codigo.
#include <SDL_log.h>
#include "StateOne.h"

StateOne::StateOne(FSMComponent *stateMachine) : BossState(stateMachine, "StateOne") {

}

void StateOne::Start() {
    SDL_Log("Starting StateOne");
    SetMaxStateTime(17.f);
}

void StateOne::Update(float deltaTime) {

}

void StateOne::HandleStateTransition(float stateTime) {

    //Se já passou o tempo de estado (ou seja, a "Prova 1" acabou)
    if(GetStateTime() > GetMaxStateTime()) {

    }
}


