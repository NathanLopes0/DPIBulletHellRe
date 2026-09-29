//
// A ponte entre as regras lidas de arquivo e o motor.
//

#pragma once

#include <functional>
#include <string>

#include "RegrasDeAtaque.h"
#include "../Actors/Projectile.h"

/**
 * @brief Devolve um configurator que aplica um conjunto de regras de
 * Assets/Attacks/regras.json.
 *
 * Substitui o lambda escrito a mao no AddAttackPattern:
 *
 *     boss->AddAttackPattern(STATE, estrategia, params, cooldown,
 *                            ConfiguratorDeArquivo("julio_fase2"));
 *
 * O arquivo e lido UMA vez, na primeira chamada. Conjunto inexistente ou
 * arquivo ausente devolvem um configurator que nao faz nada, mais uma linha de
 * log dizendo o que faltou - o ataque continua disparando, so sem os behaviors.
 * Nunca derruba o jogo.
 */
std::function<void(Projectile*, int)> ConfiguratorDeArquivo(const std::string& nomeDoConjunto);

/**
 * @brief Devolve um configurator que aplica uma lista de regras JA LIDA.
 *
 * O mesmo motor de ConfiguratorDeArquivo, para quem tem as regras em maos em vez
 * de um nome em regras.json - e o caso das regras escritas em linha dentro de um
 * ataque de fases.json.
 *
 * As regras sao COPIADAS para um shared_ptr e capturadas por valor: o
 * configurator vive enquanto o chefe viver, e nao pode depender de a lista
 * original continuar existindo.
 */
std::function<void(Projectile*, int)> ConfiguratorDeRegras(const std::vector<Regra>& regras);
