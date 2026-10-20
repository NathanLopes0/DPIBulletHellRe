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

    // A FRESTA NAO GIRA MAIS, E ESTE ARQUIVO FICOU SEM NADA PARA FAZER.
    //
    // Ela girava 137 graus por disparo, para o jogador nao decorar o ciclo. O
    // desenho mudou: agora a abertura aponta SEMPRE PARA BAIXO, e o que se move
    // e o chefe - a orbita dele arrasta o corredor seguro de um lado para o
    // outro, e seguir esse corredor passou a ser a fase inteira.
    //
    // Com a fresta fixa, "para onde ela aponta" deixou de depender do disparo
    // anterior, que era a unica coisa que fases.json nao conseguia guardar.
    // Entao ela voltou para o arquivo, como "anguloCentral": 270 - o leque e
    // desenhado em torno desse angulo, e a abertura fica oposta a ele, ou seja
    // em 90 graus, que e para baixo.
    //
    // ESTA CLASSE FICA, vazia de logica como a do Vladimir: ela da um tipo ao
    // chefe e um lugar para o que precisar de C++ depois.
    Boss::CustomizeAttackParams(params, stateName);
}
