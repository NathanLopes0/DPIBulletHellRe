//
// Created by nslop on 25/09/2024.
//

#include "Boss.h"

#include <memory>
#include "../../Random.h"
#include "BossStates/StartState.h"
#include "BossStates/StateOne.h"
#include "BossStates/StateTwo.h"
#include "BossStates/StateThree.h"

#define BOSS_VELOCITY 100.f

Boss::Boss(Scene *scene)
:Actor(scene) {

    mFSMComponent = new FSMComponent(this);
    mAtkTimer = 0;

    new StartState(mFSMComponent);
    new StateOne(mFSMComponent);
    new StateTwo(mFSMComponent);
    new StateThree(mFSMComponent);
    // new StateFinal(mFSMComponent);


    //TODO 115.0 - fazer as sprites seres 64x128, pois são mais altas do que largas.
    //TODO 115.1 - e ai mudar os raios dos colliders components de acordo (ou fazer uma sprite mais quadrada)


    mIsMoving = false;


}

void Boss::Start() {
    mFSMComponent->Start("StartState");
}

void Boss::StateActions() {
    auto stateName = mFSMComponent->GetState()->GetName();

    if(stateName == "StartState")
    {
        Movement0();
    }
    else if (stateName == "StateOne")
    {
        if(mAtkTimer < 0)
            Attack1();
        Movement1();
    }
    else if (stateName == "StateTwo")
    {
        if(mAtkTimer < 0)
            Attack2();
        Movement2();
    }
    else if (stateName == "StateThree")
    {
        if(mAtkTimer < 0)
            Attack3();
        Movement3();
    }
}

void Boss::insertComponents(DrawAnimatedComponent *newDraw, CircleColliderComponent *newCollider) {
    mDrawComponent = new DrawAnimatedComponent(*newDraw);
    mColliderComponent = new CircleColliderComponent(*newCollider);
    mRigidBodyComponent = new RigidBodyComponent(this);
}

void Boss::insertAttackStrategies(const std::vector<AttackStrategy*>& newStrategies){
    mAtkStrategies = newStrategies;
}

void Boss::ResetAtkTimer() {
    mAtkTimer = mStateAtkTimers[mFSMComponent->GetState()->GetName()];
}

void Boss::BaseMovement() {
    float chance = Random::GetFloat();
    if(!mIsMoving) {
        if(chance < 0.5)
            mRigidBodyComponent->SetVelocity(Vector2(BOSS_VELOCITY, 0));
        else
            mRigidBodyComponent->SetVelocity(Vector2(-BOSS_VELOCITY, 0));
        mIsMoving = true;
        return;
    }

    if(GetPosition().x <= (float)mDrawComponent->GetSpriteWidth()/2 + 64) {
        mRigidBodyComponent->SetVelocity(Vector2(BOSS_VELOCITY, 0));
    }
    if(GetPosition().x >= (float)mScene->GetGame()->GetWindowWidth() - (float)mDrawComponent->GetSpriteWidth()/2 - 64) {
        mRigidBodyComponent->SetVelocity(Vector2(-BOSS_VELOCITY, 0));
    }

    if(chance < 0.005) {
        mRigidBodyComponent->SetVelocity(mRigidBodyComponent->GetVelocity() * -1);
    }

}

Boss::~Boss() = default;