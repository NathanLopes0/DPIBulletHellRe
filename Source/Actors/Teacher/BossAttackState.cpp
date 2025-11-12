//
// Created by gensh on 01/11/2025.
//

#include "BossAttackState.h"
#include "../../Components/AIComponents/FSMComponent.h"

BossAttackState::BossAttackState(FSMComponent* fsm,
                                 const std::string& name,
                                 float duration,
                                 const std::string& nextStateName)
    : FSMState(fsm, name),
      mDuration(duration),
      mNextStateName(nextStateName)
{
}

void BossAttackState::HandleStateTransition(float stateTime)
{
    // 1. Ainda não deu o tempo? Então não faz nada.
    if (stateTime < mDuration) {
        return;
    }

    // Adicionar aqui lógicas personalizadas de mudança de estado


    if (!mNextStateName.empty()) {
        mFSM->SetState(mNextStateName);
    }
    // Se mNextStateName estiver vazio, ele "trava" neste estado.
}
