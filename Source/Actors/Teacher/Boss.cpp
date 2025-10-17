//
// Created by nslop on 25/09/2024.
//

#include "Boss.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Components/AIComponents/FSMComponent.h"

Boss::Boss(Scene* scene) : Actor(scene), mAtkTimer(0.0f) {

    AddComponent<RigidBodyComponent>();
    AddComponent<FSMComponent>();

}

void Boss::Start() {
    if (auto fsm = GetComponent<FSMComponent>()) {
        fsm->Start("StartState");
    }
}

void Boss::AddAttackStrategy(std::unique_ptr<IAttackStrategy> strategy) {
    mAttackStrategies.emplace_back(std::move(strategy));
}

void Boss::ExecuteAttack(const size_t index) {
    if (index < mAttackStrategies.size() && mAttackStrategies[index]) {
        mAttackStrategies[index]->Execute();
    }
}