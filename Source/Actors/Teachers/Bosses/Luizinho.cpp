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

    if (stateName == "StateOne" || stateName == "StateTwo") {
        // FASE 1 - A AMARRACAO ESTATICA E FEITA NA HORA DO DISPARO: o leque de
        // pedra sai centrado em onde o aluno esta AGORA e nao muda mais. Quem
        // andou depois ja nao esta la.
        //
        // A fase tem tres ataques e este gancho nao sabe qual dispara, entao o
        // angulo e escrito em todos. Os outros sao as rodas da amarracao
        // dinamica, CircleSpreadAttack, que ignora o angulo central.
        //
        // FASE 2 - A CAPSULA VOA para onde o aluno esta no disparo, e e por isso
        // que ela fica no caminho do tiro dele: o tiro sobe em direcao ao
        // professor, e a capsula desce pela mesma linha.
        const Vector2 d = GetDirectionToPlayer();
        params.centralAngle = Math::ToDegrees(Math::Atan2(d.y, d.x));
    }
}
