//
// Os campos que so a SprintAttack usa.
//

#pragma once

#include <string>

#include "AttackParams.h"
#include "../Sprint.h"

/// Uma sprint: quando e a revisao, e as caras da historia e do cursor.
struct SprintAttackParams : AttackParams {

    /// Quantos segundos depois do disparo vem a revisao, quando o que sobrou no quadro cai em cima
    /// do aluno.
    float revisao = Sprint::kRevisaoPadrao;

    /// As animacoes da historia, do cursor apontando e do cursor clicando.
    /// Vazio = nao mexe.
    std::string historia;
    std::string cursor;
    std::string clique;
};
