//
// O chefe da INF 321 - Programacao Web.
//

#include "Lucas.h"

Lucas::Lucas(Scene* scene) : Boss(scene) {
}

void Lucas::OnUpdate(const float deltaTime) {
    Boss::OnUpdate(deltaTime);
}

void Lucas::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);
}
