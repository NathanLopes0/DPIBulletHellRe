//
// O chefe da INF 340 - Linguagens de Programacao.
//

#include "Luizinho.h"

Luizinho::Luizinho(Scene* scene) : Boss(scene) {
}

void Luizinho::OnUpdate(const float deltaTime) {
    Boss::OnUpdate(deltaTime);
}

void Luizinho::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);
}
