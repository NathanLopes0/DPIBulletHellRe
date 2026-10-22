//
// Os campos que so a SprintAttack usa.
//

#pragma once

#include <string>

#include "AttackParams.h"
#include "../Sprint.h"

/**
 * @brief Uma sprint: quando e a revisao, e as caras da historia e do cursor.
 *
 * Mesmo padrao da SerpenteAttackParams: o bloco "sprint" e OPCIONAL em
 * fases.json, e a ponte constroi esta struct para todo ataque SprintAttack.
 */
struct SprintAttackParams : AttackParams {

    /// Quantos segundos depois do disparo vem a revisao, quando o que sobrou no
    /// quadro cai em cima do aluno. Ver Sprint::Planejar: ela nunca vem antes
    /// de as historias chegarem as vagas.
    float revisao = Sprint::kRevisaoPadrao;

    /// As animacoes da historia, do cursor apontando e do cursor clicando.
    /// Vazio = nao mexe.
    std::string historia;
    std::string cursor;
    std::string clique;
};
