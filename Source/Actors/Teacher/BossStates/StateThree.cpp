//
// Created by nslop on 15/12/2024.
//


// TODO 25.0 - Tirar o include <SDL_log.h>, estou usando só pra debugar o código
#include <SDL_log.h>
#include "../Boss.h"
#include "StateThree.h"

StateThree::StateThree(FSMComponent *stateMachine) : BossState(stateMachine, "StateThree") {

}

void StateThree::Start() {
    SDL_Log("Starting StateThree");
    SetMaxStateTime(1.f);
}

void StateThree::Update(float deltaTime) {

}

void StateThree::HandleStateTransition(float stateTime) {

    if(GetStateTime() > GetMaxStateTime()) {
        mBoss->GetScene()->SetScene(
                Scene::SceneType::StageSelect);
    }
}