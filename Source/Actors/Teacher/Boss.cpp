//
// Created by nslop on 25/09/2024.
//

#include "Boss.h"
#include "BossStates/BossState.h"
#include "BossStates/StartState.h"
#include "../../Components/ColliderComponents/CircleColliderComponent.h"

Boss::Boss(Scene *scene) : Actor(scene) {

    mRigidBodyComponent = new RigidBodyComponent(this);
    mFSMComponent = new FSMComponent(this);

    new StartState(mFSMComponent);
    // new StateOne(mFSMComponent);
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

Boss::~Boss() = default;