//
// Os campos que so a TokenAttack usa.
//

#pragma once

#include <string>
#include <vector>

#include "AttackParams.h"

/**
 * @brief A geracao de tokens: a temperatura do sorteio, os candidatos de
 * angulo e a frase que os tokens formam.
 *
 * Mesmo padrao da CaixaAttackParams: o bloco "tokens" e OPCIONAL em
 * fases.json, e a ponte constroi esta struct para todo ataque TokenAttack.
 */
struct TokenAttackParams : AttackParams {

    /// A temperatura do sorteio (Temperatura.h). Zero = sempre o mais provavel.
    float temperatura = 0.0f;

    /// Quantos angulos candidatos, e de quanto e o leque deles, em graus.
    int candidatos = 9;
    float abertura = 120.0f;

    /// As animacoes dos tokens, na ordem em que saem: a frase. Vazio = nao
    /// mexe na animacao.
    std::vector<std::string> frase;
};
