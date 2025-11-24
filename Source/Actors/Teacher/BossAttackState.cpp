//
// Created by gensh on 01/11/2025.
//

#include "BossAttackState.h"

#include "../Actor.h"
#include "../../Components/AIComponents/FSMComponent.h"
#include "../../Scenes/Battle/Battle.h"

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
        auto game = scene->GetGame();

        if (dynamic_cast<Battle*>(scene)) {
            auto selectedStage = game->GetSelectedStage();
            auto grade = game->GetGrade(selectedStage);

            if (grade > 60) {
                mNextStateName = "Approved";
            }
            else if (grade >= 40 && grade < 60) {
                mNextStateName = "StateFinal";
            }
            else {
                mNextStateName = "Reproved";
            }
        }

        if (mNextStateName == "Approved" || mNextStateName == "Reproved") {
            game->RequestSceneChange(Scene::SceneType::StageSelect);
        }
    }


    if (!mNextStateName.empty()) {
        mFSM->SetState(mNextStateName);
    }
    // Se mNextStateName estiver vazio, ele "trava" neste estado.
}
