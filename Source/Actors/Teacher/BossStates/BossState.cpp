//
// Created by nslop on 09/10/2024.
//

#include "BossState.h"
#include "../../../Components/AIComponents/FSMComponent.h"

BossState::BossState(FSMComponent *stateMachine, const std::string &name) : FSMState(stateMachine, name) {

    mStateTime = 0;
    mBoss = reinterpret_cast<Boss *>(mFSM->GetOwner());

}

void BossState::Update(float deltaTime) {

}

void BossState::SetState(FSMState *pState) {

}
