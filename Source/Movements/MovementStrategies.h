#pragma once

#include "../Math.h"
#include "../Components/RigidBodyComponent.h"
#include "../Actors/Teachers/Boss.h"
#include "../Random.h"

// --- INTERFACE ---
class IMovementStrategy {
public:
    virtual ~IMovementStrategy() = default;
    virtual void Initialize(Boss* boss) {}
    virtual void Update(Boss* boss, float deltaTime) = 0;
};

// --- ESTRATÉGIA 1: RANDOM WANDER ---
class RandomWanderStrategy : public IMovementStrategy {
public:
    RandomWanderStrategy(float interval, float speed, float margin = 100.0f);

    void Initialize(Boss* boss) override;
    void Update(Boss* boss, float deltaTime) override;

private:
    void PickNewTarget(Boss* boss);

    float mHoverTimer = 0.0f;

    // Constantes de Flutuação
    const float HOVER_FREQ = 2.0f;   // Rapidez da flutuação
    const float HOVER_AMP = 25.0f;   // Força da flutuação

    enum State { Moving, Idle };
    State mState;
    Vector2 mTargetPos;
    float mChangeInterval;
    float mMoveSpeed;
    float mMargin;
    float mWaitTimer;
    bool mIsMoving;
};

// --- ESTRATÉGIA 2: HOVER ABOVE PLAYER (Renomeado corretamente) ---
class HoverAbovePlayerStrategy : public IMovementStrategy {
public:
    HoverAbovePlayerStrategy(float speed = 150.0f, float yLevel = 150.0f);

    void Update(Boss* boss, float deltaTime) override;

private:
    float mSpeed;
    float mPreferredY;

    float mHoverTimer = 0.0f;
};

// --- ESTRATÉGIA 3: STATIC CENTER ---
class GoToCenterStrategy : public IMovementStrategy {
public:
    void Initialize(Boss* boss) override;
    void Update(Boss* boss, float deltaTime) override;
private:
    bool mReached = false;
};