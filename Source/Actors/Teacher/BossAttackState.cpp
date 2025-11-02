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

    // 3. Se nenhuma condição especial foi atingida (ou este não é o "Phase3"),
    //    apenas vá para o próximo estado padrão.
    if (!mNextStateName.empty()) {
        mFSM->SetState(mNextStateName);
    }
    // Se mNextStateName estiver vazio, ele "trava" neste estado.
}
