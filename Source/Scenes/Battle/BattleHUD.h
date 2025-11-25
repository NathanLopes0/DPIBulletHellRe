//
// Created by gensh on 11/11/2025.
//

#pragma once
#include <vector>


class Actor;
class Scene;
class ProgressBarComponent;

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

    // --- SISTEMA DE ORBES VISUAIS ---
    void UpdateVisualOrbs(float deltaTime);

    std::vector<Actor*> mVisualOrbs; // Apenas ponteiros observadores (a Scene é dona)
    float mPulseTimer = 0.0f;
    const float ORB_SMOOTH_SPEED = 20.0f;
    const float ORB_DISTANCE = 45.0f;

};
