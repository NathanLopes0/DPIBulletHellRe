//
// Created by gensh on 01/11/2025.
//

#include "BossAttackState.h"

#include "../Actor.h"
#include "../../Components/AIComponents/FSMComponent.h"
#include "../../Scenes/Battle/Battle.h"
#include "../../Nota.h"

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
    if (mName == "StateThree") {
        const auto scene = mFSM->GetOwner()->GetScene();

        if (const auto battle = dynamic_cast<Battle*>(scene)) {

            // A nota vem do Battle: e a da batalha em curso, nao a gravada.
            //
            // A REGRA MORA EM Nota::AposTerceiraFase, e nao nos numeros soltos
            // que estavam aqui. Ela ja mudou uma vez - a nota cheia passou a
            // levar ao teste final - e uma condicao escrita a mao no meio de um
            // dynamic_cast e o pior lugar possivel para uma regra que muda.
            //
            // SOBRE O JOGADOR DE 100: ele entra na fase final SEM a nota
            // guardada. Cada acerto custa os 6 pontos de sempre e, a partir do
            // terceiro, o teto de 99,99 fecha a volta. Isto e deliberado - a
            // fase final e um desafio, nao uma volta de honra - mas tem um preco
            // que vale saber: quem chega a 100 cedo fica exposto mais tempo que
            // quem chega no ultimo segundo.
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
            // Aqui a batalha acaba de qualquer jeito - tanto quem veio do exame
            // quanto quem veio do teste final. So o resultado muda.
            battle->FinishBattle(battle->GetNotaAtual() >= Nota::kNotaAprovacao);
        }
    }


    // Se mNextStateName estiver vazio, ele "trava" neste estado.
    if (!mNextStateName.empty()) {
        mFSM->SetState(mNextStateName);
    }

}
