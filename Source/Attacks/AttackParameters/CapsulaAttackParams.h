//
// Os campos que so a CapsulaAttack usa.
//

#pragma once

#include <string>

#include "AttackParams.h"

/**
 * @brief Uma capsula: quando ela abre, com que velocidade as letras saem, e
 * com que cara elas ficam.
 *
 * Mesmo padrao do CaixaAttackParams: o bloco "capsula" e OPCIONAL em
 * fases.json, a ponte constroi esta struct para todo ataque CapsulaAttack, e
 * um campo ausente fica com o padrao abaixo.
 */
struct CapsulaAttackParams : AttackParams {

    /// Segundos de voo fechada antes de abrir.
    float abre = 1.1f;

    /// O modulo das letras do anel, depois de aberta.
    float velocidadeDaAbertura = 150.0f;

    /// A animacao das letras depois de aberta. Vazio = continuam com a cara da
    /// casca, o que so serve para teste: e a troca que diz ao aluno "abriu".
    std::string escreve;
};
