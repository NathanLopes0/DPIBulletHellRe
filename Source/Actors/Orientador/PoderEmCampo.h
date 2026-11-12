//
// O poder do orientador em campo: um objeto por poder.
//

#pragma once

#include <memory>

#include "../../Motor/Math.h"
#include "../../Regras/Poder.h"

class Battle;
class Orientador;

/// O que cada poder faz na batalha (padrao strategy). Por que um objeto por poder.
class PoderEmCampo {
public:
    virtual ~PoderEmCampo() = default;

    /// O poder do tipo 't', ou nulo para Poder::Tipo::Nenhum.
    static std::unique_ptr<PoderEmCampo> Do(Poder::Tipo t);

    /// O botao 3: o efeito comeca.
    virtual void Soltar(Orientador& o, Battle& batalha, const Vector2& aluno, const Vector2& alvo) = 0;

    /// Todo quadro, desde o comeco da batalha.
    virtual void Atualizar(Orientador& o, Battle& batalha, float deltaTime, bool ativo) {}
};
