//
// Created by gensh on 11/11/2025.
//

#pragma once


class Actor;
class Scene;
class ProgressBarComponent; // TODO vem ai

class BattleHUD {

public:
    BattleHUD(Scene* scene);

    void OnUpdate(float deltaTime);
    void SetTimeBar(Actor* barActor, ProgressBarComponent* barComp);

    void ResetTimeBar(float newTotalTime);

private:

    Scene* mOwner;
    Actor* mTimeBarActor = nullptr;
    ProgressBarComponent* mTimeBarComp = nullptr;

    float mCurrentStateTime;
    float mTotalStateTime;

};
