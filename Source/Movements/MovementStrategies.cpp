//
// Created by gensh on 24/11/2025.
//

#include "MovementStrategies.h"
#include "../Scenes/Battle/Battle.h"
#include "../Actors/Player/Player.h"

// =========================================================
// RANDOM WANDER STRATEGY
// =========================================================

RandomWanderStrategy::RandomWanderStrategy(float interval, float speed, float margin)
    : mState(Idle), mChangeInterval(interval), mMoveSpeed(speed),
      mMargin(margin), mWaitTimer(0.0f), mIsMoving(false)
{
}
void RandomWanderStrategy::Initialize(Boss* boss) {
    PickNewTarget(boss);
}
void RandomWanderStrategy::Update(Boss* boss, float deltaTime) {
    auto rb = boss->GetComponent<RigidBodyComponent>();
    if (!rb) return;

    mHoverTimer += deltaTime;

    if (mState == Moving) {
        Vector2 toTarget = mTargetPos - boss->GetPosition();
        float dist = toTarget.Length();

        if (dist < 10.0f) {
            mState = Idle;

            // Ex: Se configurou 3.0s, ele espera entre 2.4s e 3.6s.
            float minWait = mChangeInterval * 0.8f;
            float maxWait = mChangeInterval * 1.2f;
            mWaitTimer = Random::GetFloatRange(minWait, maxWait);

            rb->SetVelocity(Vector2::Zero);
        } else {
            toTarget.Normalize();
            float slowFactor = Math::Min(1.0f, dist / 150.0f);
            rb->SetVelocity(toTarget * (mMoveSpeed * slowFactor));
        }
    }
    else if (mState == Idle) {
        mWaitTimer -= deltaTime;
        const float hoverVy = std::cos(mHoverTimer * HOVER_FREQ) * HOVER_AMP;
        rb->SetVelocity(Vector2(0.0f, hoverVy));
        if (mWaitTimer <= 0.0f) {
            PickNewTarget(boss);
        }
    }
}
void RandomWanderStrategy::PickNewTarget(Boss* boss) {
    auto w = static_cast<float>(boss->GetScene()->GetGame()->GetWindowWidth());
    auto h = static_cast<float>(boss->GetScene()->GetGame()->GetWindowHeight());

    mTargetPos = Vector2(
        Random::GetFloatRange(mMargin, w - mMargin),
        Random::GetFloatRange(mMargin, h / 2.5f)
    );
    mState = Moving;
}

// =========================================================
// HOVER ABOVE PLAYER STRATEGY
// =========================================================

HoverAbovePlayerStrategy::HoverAbovePlayerStrategy(float speed, float yLevel)
    : mSpeed(speed), mPreferredY(yLevel)
{
}

void HoverAbovePlayerStrategy::Update(Boss* boss, float deltaTime) {
    auto rb = boss->GetComponent<RigidBodyComponent>();

    mHoverTimer += deltaTime;

    float targetX = boss->GetPosition().x;

    if (const auto battle = dynamic_cast<Battle*>(boss->GetScene())) {
        if (const auto player = battle->GetPlayer()) {
            targetX = player->GetPosition().x;
        }
    }

    Vector2 currentPos = boss->GetPosition();
    float hoverOffset = Math::Sin(mHoverTimer * 2.f) * 15.f;

    Vector2 desiredPos = Vector2(targetX, mPreferredY + hoverOffset);

    Vector2 toTarget = desiredPos - currentPos;

    Vector2 desiredVel = toTarget;
    if(desiredVel.Length() > 1.0f) desiredVel.Normalize();
    desiredVel *= mSpeed;

    Vector2 currentVel = rb->GetVelocity();
    Vector2 newVel = Vector2::Lerp(currentVel, desiredVel, 5.0f * deltaTime); // Aumentei para 5.0f pra ficar mais ágil

    rb->SetVelocity(newVel);
}

// =========================================================
// GO TO CENTER STRATEGY
// =========================================================

void GoToCenterStrategy::Initialize(Boss* boss) {
    mReached = false;
}

void GoToCenterStrategy::Update(Boss* boss, float deltaTime) {
    auto rb = boss->GetComponent<RigidBodyComponent>();
    if (mReached) {
        rb->SetVelocity(Vector2::Zero);
        return;
    }

    auto w = static_cast<float>(boss->GetScene()->GetGame()->GetWindowWidth());
    auto h = static_cast<float>(boss->GetScene()->GetGame()->GetWindowHeight());
    auto center = Vector2(w / 2.0f, h / 5.0f);

    Vector2 toTarget = center - boss->GetPosition();
    float dist = toTarget.Length();

    if (dist < 5.0f) {
        mReached = true;
        rb->SetVelocity(Vector2::Zero);
    } else {
        toTarget.Normalize();
        float slowFactor = Math::Min(1.0f, dist / 100.0f);
        rb->SetVelocity(toTarget * (200.0f * slowFactor));
    }
}