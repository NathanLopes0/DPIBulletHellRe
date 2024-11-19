//
// Created by nslop on 25/09/2024.
//

#include "Boss.h"
#include "BossStates/BossState.h"
#include "BossStates/StartState.h"

Boss::Boss(Scene *scene) : Actor(scene) {

    mRigidBodyComponent = new RigidBodyComponent(this);
    mFSMComponent = new FSMComponent(this);

    new StartState(mFSMComponent);
    // new StateOne(mFSMComponent);
    // new StateTwo(mFSMComponent);
    // new StateThree(mFSMComponent);
    // new StateFinal(mFSMComponent);


}

void Boss::Start() {
    mFSMComponent->Start("StartState");
}

Boss::~Boss() = default;