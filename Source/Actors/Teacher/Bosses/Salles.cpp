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

    if (stateName == "StateOne") {
        if (const auto chance = Random::GetFloatRange(0.0f, 1.0f); chance < 0.3f) {
            params.centralAngle = Math::Acos(GetDirectionToPlayer().x) * 180 / Math::Pi;
        }
        else {
            params.centralAngle = Random::GetFloatRange(45.f, 135.f);
        }

    }

    if (stateName == "StateTwo") {
        if (const auto chance = Random::GetFloatRange(0.0f, 1.0f); chance < 0.3f) {
            params.centralAngle = Math::Acos(GetDirectionToPlayer().x) * 180 / Math::Pi;
        }
        else {
            params.centralAngle = Random::GetFloatRange(45.f, 135.f);
        }
    }

    if (stateName == "StateThree") {
        if (const auto chance = Random::GetFloatRange(0.0f, 1.0f); chance < 0.3f) {
            params.centralAngle = Math::Acos(GetDirectionToPlayer().x) * 180 / Math::Pi;
        }
        else {
            params.centralAngle = Random::GetFloatRange(0.f, 180.f);
        }
    }

    if (stateName == "StateFinal") {
        params.centralAngle = Random::GetFloatRange(0.f, 180.f);
    }
}




