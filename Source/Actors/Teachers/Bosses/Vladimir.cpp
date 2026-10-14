//
// O chefe da INF 331 - Linguagens Formais e Automatos.
//

#include "Vladimir.h"

Vladimir::Vladimir(Scene* scene) : Boss(scene) {
}

void Vladimir::OnUpdate(const float deltaTime) {
    Boss::OnUpdate(deltaTime);
}

void Vladimir::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {

    // NADA AQUI, E DE PROPOSITO. CustomizeAttackParams e o unico pedaco do
    // balanceamento que ainda vive em C++, e e onde o angulo de um ataque pode
    // ser sobrescrito em tempo de execucao. Os padroes do Vladimir sao
    // deterministicos - uma palavra tem a forma que tem -, entao nenhum deles
    // precisa de sorteio no codigo: tudo cabe em fases.json e regras.json.
    Boss::CustomizeAttackParams(params, stateName);
}
