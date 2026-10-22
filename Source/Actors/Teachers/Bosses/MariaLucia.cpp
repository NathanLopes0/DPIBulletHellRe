//
// A chefe da INF 221 - Engenharia de Software.
//

#include "MariaLucia.h"

MariaLucia::MariaLucia(Scene* scene) : Boss(scene) {
}

void MariaLucia::OnUpdate(const float deltaTime) {
    Boss::OnUpdate(deltaTime);
}
