//
// Created by nslop on 25/09/2024.
//

#include "Salles.h"
#include "../../../Random.h"
#include "../../../Components/ColliderComponents/CircleColliderComponent.h"


Salles::Salles(Scene *scene) : Boss(scene)
{

}

void Salles::OnUpdate(float deltaTime) {

    mAttackCooldown -= deltaTime;
    if (mAttackCooldown <= 0.0f) {
        Attack();
    }
}

void Salles::CustomizeAttackParams(AttackParams &params, const std::string &stateName) {
    Boss::CustomizeAttackParams(params, stateName);

    if (stateName == "StateTwo") {
        params.centralAngle = Random::GetFloatRange(45.f, 135.f);
    }
}




