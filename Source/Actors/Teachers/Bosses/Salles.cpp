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
    Boss::OnUpdate(deltaTime);
}

void Salles::CustomizeAttackParams(AttackParams &params, const std::string &stateName) {
    Boss::CustomizeAttackParams(params, stateName);

    // Mesmo conserto de Andre::CustomizeAttackParams: Math::Acos(dir.x) so
    // devolve [0, 180] e perdia o sinal de y, fazendo o boss mirar para baixo
    // quando o player estava acima. Calculado uma vez, em vez de tres.
    const Vector2 dirToPlayer = GetDirectionToPlayer();
    const float angleToPlayer = Math::ToDegrees(Math::Atan2(dirToPlayer.y, dirToPlayer.x));

    if (stateName == "StateOne") {
        if (const auto chance = Random::GetFloatRange(0.0f, 1.0f); chance < 0.3f) {
            params.centralAngle = angleToPlayer;
        }
        else {
            params.centralAngle = Random::GetFloatRange(45.f, 135.f);
        }

    }

    if (stateName == "StateTwo") {
        if (const auto chance = Random::GetFloatRange(0.0f, 1.0f); chance < 0.3f) {
            params.centralAngle = angleToPlayer;
        }
        else {
            params.centralAngle = Random::GetFloatRange(45.f, 135.f);
        }
    }

    if (stateName == "StateThree") {
        if (const auto chance = Random::GetFloatRange(0.0f, 1.0f); chance < 0.3f) {
            params.centralAngle = angleToPlayer;
        }
        else {
            params.centralAngle = Random::GetFloatRange(0.f, 180.f);
        }
    }

    if (stateName == "StateFinal") {
        params.centralAngle = Random::GetFloatRange(0.f, 180.f);
    }
}




