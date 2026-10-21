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

    if (stateName == "StateOne") {
        // A AMARRACAO ESTATICA E FEITA NA HORA DO DISPARO: o leque de pedra sai
        // centrado em onde o aluno esta AGORA e nao muda mais. Quem andou depois
        // ja nao esta la.
        //
        // A fase tem dois ataques e este gancho nao sabe qual dispara, entao o
        // angulo e escrito nos dois. O outro e a roda da amarracao dinamica, uma
        // CircleSpreadAttack, que ignora o angulo central.
        const Vector2 d = GetDirectionToPlayer();
        params.centralAngle = Math::ToDegrees(Math::Atan2(d.y, d.x));
    }
}
