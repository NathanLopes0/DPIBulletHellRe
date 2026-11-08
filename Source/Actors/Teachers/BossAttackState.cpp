//
// Created by gensh on 01/11/2025.
//

#include "BossAttackState.h"
#include "Boss.h"

#include "../Actor.h"
#include "../../Components/AIComponents/FSMComponent.h"
#include "../../Scenes/Battle/Battle.h"
#include "../../Regras/Nota.h"

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
    // 1.
    if (stateTime < mDuration) {
        return;
    }

    // O segundo corpo nao transiciona, e nao decide a batalha.
    if (const auto corpo = dynamic_cast<Boss*>(mFSM->GetOwner()); corpo && corpo->EhClone()) {
        return;
    }

    // Adicionar aqui lógicas personalizadas de mudança de estado.
    if (mName == "StateThree") {
        const auto scene = mFSM->GetOwner()->GetScene();

        if (const auto battle = dynamic_cast<Battle*>(scene)) {

            // A nota vem do Battle: e a da batalha em curso, nao a gravada.
            switch (Nota::AposTerceiraFase(battle->GetNotaAtual())) {

                case Nota::Desfecho::VaiParaFinal:
                    mNextStateName = "StateFinal";
                    break;

                case Nota::Desfecho::Aprovado:
                    battle->FinishBattle(true);
                    break;

                case Nota::Desfecho::Reprovado:
                    battle->FinishBattle(false);
                    break;
            }
        }
    }

    if (mName == "StateFinal") {
        const auto scene = mFSM->GetOwner()->GetScene();

        if (const auto battle = dynamic_cast<Battle*>(scene)) {
            // Aqui a batalha acaba de qualquer jeito.
            battle->FinishBattle(battle->GetNotaAtual() >= Nota::kNotaAprovacao);
        }
    }

    // Se mNextStateName estiver vazio, ele "trava" neste estado.
    if (!mNextStateName.empty()) {
        mFSM->SetState(mNextStateName);
    }

}
