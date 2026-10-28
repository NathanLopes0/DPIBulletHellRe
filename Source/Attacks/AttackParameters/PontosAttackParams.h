//
// Os campos que so a PontosAttack usa.
//

#pragma once

#include <string>
#include <vector>

#include "AttackParams.h"

/**
 * @brief Um disparo de varios pontos ao mesmo tempo: de qual POSTO, se cada
 * ponto mira o aluno, e - posto pelo Boss na hora do disparo - os pontos e
 * onde o aluno esta.
 *
 * O bloco "pontos" e OBRIGATORIO em fases.json: uma PontosAttack sem posto nao
 * teria de onde atirar.
 */
struct PontosAttackParams : AttackParams {

    /// O nome do posto (FasesDeAtaque.h, PostoExiste). Do arquivo.
    std::string posto;

    /// Cada ponto mira o aluno; sem mirar, todos saem em centralAngle.
    bool mirar = false;

    /// Mirando, mira onde o aluno VAI ESTAR (RumoPrevendo, PathAim.h).
    bool prever = false;

    // ---- POSTOS PELO BOSS a cada disparo (Boss::ExecuteAttack) ----------

    /// De onde sai cada leque: Boss::PontosDoPosto(posto).
    std::vector<Vector2> pontos;

    /// Onde o aluno da vez esta e como anda, se houver aluno.
    Vector2 alvo = Vector2::Zero;
    Vector2 velocidadeDoAlvo = Vector2::Zero;
    bool temAlvo = false;
};
