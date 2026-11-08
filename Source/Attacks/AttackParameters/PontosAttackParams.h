//
// Os campos que so a PontosAttack usa.
//

#pragma once

#include <string>
#include <vector>

#include "AttackParams.h"

/// Um disparo de varios pontos ao mesmo tempo: de qual posto, se cada ponto mira o aluno, e - posto
/// pelo Boss na hora do disparo.
struct PontosAttackParams : AttackParams {

    /// O nome do posto (FasesDeAtaque.h, PostoExiste). Do arquivo.
    std::string posto;

    /// Cada ponto mira o aluno; sem mirar, todos saem em centralAngle.
    bool mirar = false;

    /// Mirando, mira onde o aluno vai estar (RumoPrevendo, PathAim.h).
    bool prever = false;

    // ---- Postos pelo boss a cada disparo (Boss::ExecuteAttack) ----

    /// De onde sai cada leque: Boss::PontosDoPosto(posto).
    std::vector<Vector2> pontos;

    /// Onde o aluno da vez esta e como anda, se houver aluno.
    Vector2 alvo = Vector2::Zero;
    Vector2 velocidadeDoAlvo = Vector2::Zero;
    bool temAlvo = false;
};
