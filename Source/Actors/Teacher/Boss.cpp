//
// Created by nslop on 25/09/2024.
//

#include "Boss.h"
#include "BossStates/StartState.h"
#include "BossStates/StateOne.h"
#include "../../Components/ColliderComponents/CircleColliderComponent.h"

Boss::Boss(Scene *scene) : Actor(scene) {

    mRigidBodyComponent = new RigidBodyComponent(this);
    mFSMComponent = new FSMComponent(this);

    new StartState(mFSMComponent);
    new StateOne(mFSMComponent);
    // new StateTwo(mFSMComponent);
    // new StateThree(mFSMComponent);
    // new StateFinal(mFSMComponent);


    //metade do tamanho da sprite, pq é o raio. Não dá pra colocar GetSpriteWidth/Height pq ainda não temos a sprite,
    //pois ela é colocada no construtor externo(do boss especifico), e esse construtor mais interno acontece primeiro.
    mColliderComponent = new CircleColliderComponent(this, 64);

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
        Attack1();
        Movement1();
    }
    else if (stateName == "StateTwo")
    {
        Attack2();
        Movement2();
    }
    else if (stateName == "StateThree")
    {
        Attack3();
        Movement3();
    }
}

Boss::~Boss() = default;