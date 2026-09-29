//
// A ponte entre as fases lidas de arquivo e o motor.
//

#pragma once

#include <string>

class Boss;
class FSMComponent;

/**
 * @brief Monta as fases de um chefe a partir de Assets/Attacks/fases.json.
 *
 * Substitui as quatro funcoes ConfigureStateOne..Final de uma fabrica:
 *
 *     void JulioFactory::ConfigureAttacksAndFSM(Boss* boss) {
 *         auto fsm = boss->GetComponent<FSMComponent>();
 *         if (!ConfigurarFasesDeArquivo(boss, fsm, "julio")) {
 *             ConfigureStateOne(boss, fsm);   // reserva em C++
 *             ...
 *         }
 *         boss->SetInitialState("StateOne");
 *     }
 *
 * Registra, para cada fase do conjunto: os ataques (AddAttackPattern), o estado
 * na maquina de estados (BossAttackState) e a estrategia de movimento.
 *
 * O arquivo e lido UMA vez, na primeira chamada.
 *
 * Devolve FALSE, sem ter registrado NADA, quando o conjunto nao existe ou nao
 * passa na validacao de transicoes - para que a fabrica possa cair na
 * configuracao de reserva em C++. E tudo ou nada de proposito: um conjunto pela
 * metade e pior que nenhum, porque deixa o chefe atacando numa fase e congelado
 * na seguinte, o que parece bug de jogo e nao erro de arquivo.
 *
 * Um ataque solto que pede uma fabrica de projeteis inexistente e a excecao: ele
 * e ignorado com uma linha de log e o resto da fase continua valendo, do mesmo
 * jeito que uma forma ausente vira uma reta.
 */
bool ConfigurarFasesDeArquivo(Boss* boss, FSMComponent* fsm, const std::string& nomeDoConjunto);
