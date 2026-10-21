//
// O chefe da INF 310 - Programacao Concorrente e Distribuida.
//

#include "Vitor.h"

#include "../../../Attacks/AttackParameters/AttackParams.h"

#include <cmath>

Vitor::Vitor(Scene* scene) : Boss(scene) {
}

void Vitor::OnUpdate(const float deltaTime) {
    Boss::OnUpdate(deltaTime);
}

void Vitor::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {

    Boss::CustomizeAttackParams(params, stateName);

    // A FRESTA ANDA, MAS SO PELA METADE DE BAIXO DA TELA.
    //
    // E a unica coisa do Vitor que nao cabe em dados: "para onde a abertura
    // aponta DESTA vez" depende da rajada anterior, e fases.json nao tem
    // memoria. O TAMANHO da fresta continua no arquivo ("angulo": 330 deixa 30
    // graus abertos); daqui sai so a DIRECAO.
    //
    // O clone da fase 3 tambem passa por aqui: ele roda os ataques de
    // StateTwo, e e outro Vitor, com a propria fresta comecando do meio.
    if (stateName == "StateTwo") {
        const float fresta = kFrestaMaisADireita + mDesvioDaFresta;

        // O leque e desenhado EM TORNO de centralAngle, entao a abertura fica
        // oposta a ele: somar 180 poe a fresta onde a variavel diz. E escrever
        // o valor inteiro, e nao somar sobre o disparo anterior - params e o
        // mesmo objeto a cada disparo (ver Boss.h).
        params.centralAngle = fresta + 180.0f;

        mDesvioDaFresta = std::fmod(mDesvioDaFresta + kPassoDaFresta, kJanelaDaFresta);
    }
}
