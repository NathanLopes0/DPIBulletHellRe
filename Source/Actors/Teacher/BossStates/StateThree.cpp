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
    SetMaxStateTime(17.f);
}

void StateThree::Update(float deltaTime) {

}

void StateThree::HandleStateTransition(float stateTime) {

    // TODO 20.0 - Mudar lógica obviamente, para ver a nota atual e decidir se ganhou, perdeu, vai fazer final etc
    if(GetStateTime() > GetMaxStateTime()) {
        mBoss->GetScene()->GetGame()->SetScene(Scene::SceneType::StageSelect);
    }
}