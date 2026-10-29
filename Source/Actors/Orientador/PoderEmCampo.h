//
// O poder do orientador em campo: um objeto por poder.
//

#pragma once

#include <memory>

#include "../../Core/Math.h"
#include "../../Gameplay/Orientador/Poder.h"

class Battle;
class Orientador;

/**
 * @class PoderEmCampo
 * @brief O que cada poder faz na batalha (padrao STRATEGY).
 *
 * POR QUE UM OBJETO POR PODER. O Orientador tinha dois switch sobre o tipo do
 * poder - um para soltar, outro para cada quadro - e guardava o estado de
 * todos eles ao mesmo tempo: a contagem da rajada, o que o HLT congelou, a
 * bala de prata, as figuras da capivara e dos agentes. Cada orientador so tem
 * UM poder, entao quase todo esse estado ficava parado, e um poder novo
 * mexia em tres lugares do Orientador. Aqui cada poder e uma classe com o seu
 * estado, e o Orientador so conhece esta interface.
 *
 * O QUE CONTINUA NO ORIENTADOR e o que e igual para todos: a carga, o anel,
 * o anuncio no quadro e quanto tempo o efeito dura (Poder::Descricao). A conta
 * de cada poder (o alcance, a orbita dos agentes, a curva do tiro guiado)
 * continua pura, em Poder.h.
 *
 * QUEM CRIA E Do(): a fabrica, o unico switch sobre o tipo que sobrou.
 */
class PoderEmCampo {
public:
    virtual ~PoderEmCampo() = default;

    /// @brief O poder do tipo 't', ou nulo para Poder::Tipo::Nenhum.
    static std::unique_ptr<PoderEmCampo> Do(Poder::Tipo t);

    /// @brief O BOTAO 3: o efeito comeca. 'aluno' e 'alvo' (o chefe) sao os
    /// de agora.
    virtual void Soltar(Orientador& o, Battle& batalha, const Vector2& aluno, const Vector2& alvo) = 0;

    /// @brief Todo quadro, desde o comeco da batalha. 'ativo': o efeito ainda
    /// dura (Poder::Descricao::duracao) - e quando ele deixa de durar que cada
    /// poder desfaz o que fez.
    virtual void Atualizar(Orientador& o, Battle& batalha, float deltaTime, bool ativo) {}
};
