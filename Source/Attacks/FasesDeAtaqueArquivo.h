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
 * E A UNICA maneira de um chefe ganhar fases. As funcoes ConfigureStateOne..Final
 * que existiam nas fabricas foram removidas: elas repetiam em C++ o mesmo
 * balanceamento que o arquivo ja descrevia, e duas fontes de verdade divergem no
 * dia em que alguem ajusta a errada.
 *
 *     void JulioFactory::ConfigureAttacksAndFSM(Boss* boss) {
 *         auto fsm = boss->GetComponent<FSMComponent>();
 *         if (!ConfigurarFasesDeArquivo(boss, fsm, "julio")) {
 *             SDL_Log("ERRO CRITICO: ...");   // o chefe fica sem ataque
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
 * passa na validacao de transicoes. E tudo ou nada de proposito: um conjunto pela
 * metade deixaria o chefe atacando numa fase e congelado na seguinte, o que
 * parece bug de jogo e nao erro de arquivo.
 *
 * FALSE AGORA SIGNIFICA UM CHEFE SEM ATAQUE, e nao mais "cai para o C++". Quem
 * impede isso de acontecer em jogo e tests/test_arquivos_de_dados.cpp, que le os
 * Assets de verdade e quebra a suite ao primeiro erro de digitacao: nome de
 * conjunto de regras que nao existe, nome de forma que nao existe, transicao
 * quebrada, fase sem ataque. E uma protecao melhor que a reserva, porque pega
 * tambem os dois erros que a reserva NUNCA pegou - nome de regra e nome de forma
 * errados deixam as fases carregarem e o jogo ficar silenciosamente errado.
 *
 * Um ataque solto que pede uma fabrica de projeteis inexistente e a excecao: ele
 * e ignorado com uma linha de log e o resto da fase continua valendo, do mesmo
 * jeito que uma forma ausente vira uma reta.
 */
bool ConfigurarFasesDeArquivo(Boss* boss, FSMComponent* fsm, const std::string& nomeDoConjunto);
