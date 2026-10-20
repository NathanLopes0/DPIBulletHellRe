//
// O chefe da INF 310 - Programacao Concorrente e Distribuida.
//

#include "Vitor.h"

#include "../../../Attacks/AttackParameters/AttackParams.h"

Vitor::Vitor(Scene* scene) : Boss(scene) {
}

void Vitor::OnUpdate(const float deltaTime) {
    Boss::OnUpdate(deltaTime);
}

void Vitor::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {

    Boss::CustomizeAttackParams(params, stateName);

    // A FRESTA GIRA, E SO NA FASE DA EXCLUSAO MUTUA.
    //
    // E a unica coisa do Vitor que nao cabe em dados: o arquivo diz o tamanho
    // do leque e quantos projeteis, mas "para onde ele aponta DESTA vez" depende
    // do disparo anterior, e fases.json nao tem memoria.
    //
    // CustomizeAttackParams roda UMA vez por rajada (Boss::ExecuteAttack), e nao
    // uma por projetil - e por isso que adiantar o angulo aqui gira o anel
    // inteiro de uma vez, em vez de espalhar os projeteis de uma mesma rajada.
    if (stateName == "StateTwo") {
        mFrestaAtual += kPassoDaFresta;
        if (mFrestaAtual >= 360.0f) mFrestaAtual -= 360.0f;

        // O leque e desenhado em torno de centralAngle, entao a FRESTA fica
        // oposta a ele: somar 180 poe a abertura onde este campo aponta, que e
        // como o nome "fresta" se le.
        params.centralAngle = mFrestaAtual + 180.0f;
    }
}
