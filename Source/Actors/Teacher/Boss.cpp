//
// Created by nslop on 25/09/2024.
//

#include "Boss.h"
#include "BossStates/StartState.h"
#include "BossStates/StateOne.h"

Boss::Boss(Scene *scene, const std::string& spritePath, const std::string& dataPath)
:Actor(scene) {

    mRigidBodyComponent = new RigidBodyComponent(this);
    mFSMComponent = new FSMComponent(this);
    mAtkTimer = 0;

    new StartState(mFSMComponent);
    new StateOne(mFSMComponent);
    // new StateTwo(mFSMComponent);
    // new StateThree(mFSMComponent);
    // new StateFinal(mFSMComponent);

    //todos os bosses terão sprite, logo, declarei aqui. A divisão do Collider por 2.2 é pq por 2 tava pegando fora da sprite,
    //ainda tá pegando fora na verdade, mas não tanto. Talvez eu devesse fazer a sprite ser com mais altura do que largura,
    //ao invés de um quadrado...

    //TODO 115.0 - fazer as sprites seres 64x128, pois são mais altas do que largas.
    //TODO 115.1 - e ai mudar os raios dos colliders components de acordo (ou fazer uma sprite mais quadrada)

    mDrawComponent = new DrawAnimatedComponent(this, spritePath, dataPath);
    mColliderComponent = new CircleColliderComponent(this, (float)mDrawComponent->GetSpriteWidth() / 2.2f);


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