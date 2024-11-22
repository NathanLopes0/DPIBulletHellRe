//
// Created by nslop on 25/09/2024.
//

#include "Boss.h"
#include "BossStates/StartState.h"
#include "BossStates/StateOne.h"

Boss::Boss(Scene *scene) : Actor(scene) {

    mRigidBodyComponent = new RigidBodyComponent(this);
    mFSMComponent = new FSMComponent(this);
    mAtkTimer = 0;

    new StartState(mFSMComponent);
    new StateOne(mFSMComponent);
    // new StateTwo(mFSMComponent);
    // new StateThree(mFSMComponent);
    // new StateFinal(mFSMComponent);

    mDrawComponent = nullptr;
    mColliderComponent = nullptr;


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

Boss::~Boss() = default;