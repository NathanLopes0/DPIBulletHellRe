//
// Os campos que so a SerpenteAttack usa.
//

#pragma once

#include <memory>
#include <string>

#include "AttackParams.h"
#include "../Rastro.h"

/**
 * @brief Uma serpente: o espaco entre os gomos, o quanto a cabeca vira, por
 * quanto tempo, e as caras da cabeca e do corpo.
 *
 * Mesmo padrao do CaixaAttackParams: o bloco "serpente" e OPCIONAL em
 * fases.json, e a ponte constroi esta struct para todo ataque SerpenteAttack.
 */
struct SerpenteAttackParams : AttackParams {

    /// Pixels de caminho entre um gomo e o seguinte.
    float espaco = 24.0f;

    /// Quanto a cabeca vira, em radianos por segundo. A curva mais fechada que
    /// ela faz tem raio velocidade / giro.
    float giro = 1.5f;

    /// Por quantos segundos a cabeca persegue (e obedece ao goto) antes de
    /// seguir reta para fora da tela.
    float duracao = 4.5f;

    /// As animacoes da cabeca e dos gomos. Vazio = nao mexe.
    std::string cabeca;
    std::string corpo;

    /// O goto de quem dispara. Quem o poe aqui e o professor, em
    /// CustomizeAttackParams; sem ele, a serpente so persegue.
    std::shared_ptr<const Rastro::Desvio> desvio;
};
