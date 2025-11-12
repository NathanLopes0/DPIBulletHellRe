//
// Created by gensh on 11/11/2025.
//

#include "BattleHUD.h"

#include "../../Actors/Actor.h"
#include "../../Components/DrawComponents/ProgressBarComponent.h"



BattleHUD::BattleHUD(Scene *scene)
    :mOwner(scene),
    mCurrentStateTime(0.0f),
    mTotalStateTime(17.f)
{

}

void BattleHUD::OnUpdate(float deltaTime) {
    mCurrentStateTime += deltaTime;
    float progress = 1.0f - (mCurrentStateTime / mTotalStateTime);

    if (mTimeBarComp != nullptr) {
        mTimeBarComp->SetProgress(progress);
    }
}

// ----- Metodo de injeção para Battle chamar ----- //
void BattleHUD::SetTimeBar(Actor *barActor, ProgressBarComponent *barComp) {
    mTimeBarActor = barActor;
    mTimeBarComp = barComp;
}

void BattleHUD::ResetTimeBar(const float newTotalTime) {
    mCurrentStateTime = 0.0f;

    if (newTotalTime > 0) {
        mTotalStateTime = newTotalTime;
    }
    else {
        mTotalStateTime = 1.0f;
    }


    if (mTimeBarActor) {
        // Enche a barra novamente
        mTimeBarActor->GetComponent<ProgressBarComponent>()->SetProgress(1.0f);
    }
}


