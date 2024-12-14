//
// Created by nslop on 25/09/2024.
//

#include "Boss.h"

#include <utility>
#include <memory>
#include "BossStates/StartState.h"
#include "BossStates/StateOne.h"
#include "BossStates/StateTwo.h"

Boss::Boss(Scene *scene)
:Actor(scene) {

    mFSMComponent = new FSMComponent(this);
    mAtkTimer = 0;

    new StartState(mFSMComponent);
    new StateOne(mFSMComponent);
    new StateTwo(mFSMComponent);
    // new StateThree(mFSMComponent);
    // new StateFinal(mFSMComponent);


    //TODO 115.0 - fazer as sprites seres 64x128, pois são mais altas do que largas.
    //TODO 115.1 - e ai mudar os raios dos colliders components de acordo (ou fazer uma sprite mais quadrada)




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

void Boss::insertComponents(DrawAnimatedComponent *newDraw, CircleColliderComponent *newCollider,
                            RigidBodyComponent *newRigid) {
    mDrawComponent = new DrawAnimatedComponent(*newDraw);
    mColliderComponent = new CircleColliderComponent(*newCollider);
    mRigidBodyComponent = new RigidBodyComponent(this);
}

void Boss::insertAttackStrategies(const std::vector<AttackStrategy*>& newStrategies){
    mAtkStrategies = newStrategies;
}

Boss::~Boss() = default;