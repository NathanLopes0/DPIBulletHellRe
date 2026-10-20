//
// A chefe da INF 221 - Engenharia de Software.
//

#include "MariaLucia.h"

MariaLucia::MariaLucia(Scene* scene) : Boss(scene) {
}

void MariaLucia::OnUpdate(const float deltaTime) {
    Boss::OnUpdate(deltaTime);
}

void MariaLucia::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {

    // NADA AQUI AINDA. CustomizeAttackParams e o unico pedaco do balanceamento
    // que vive em C++, e e o lugar de tudo que depende do DISPARO ANTERIOR -
    // um arquivo de fases nao tem memoria. E dai que vem a fresta giratoria do
    // Vitor.
    //
    // O escopo que cresce a cada rajada mora aqui quando for a hora; por ora a
    // chefe inteira cabe em dados, e enquanto couber e melhor que fique la.
    Boss::CustomizeAttackParams(params, stateName);
}
