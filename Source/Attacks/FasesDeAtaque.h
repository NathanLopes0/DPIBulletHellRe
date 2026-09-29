//
// Fases de ataque de um chefe, lidas de texto JSON.
//

#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "RegrasDeAtaque.h"

/**
 * CAMADA PURA (mesma disciplina de PathAim, PathShapes, RegrasDeAtaque e
 * Progresso)
 *
 * Nao abre arquivo, nao escreve log, nao conhece Boss, FSMComponent, AttackParams
 * nem SDL. Transforma TEXTO em descricoes e aponta os problemas. Quem monta os
 * objetos de verdade e FasesDeAtaqueArquivo.cpp, a ponte fina com o motor.
 */

/**
 * @brief Os campos que SO a BaloonAttack usa.
 *
 * A BaloonAttack e a unica estrategia que nao se contenta com AttackParams: ela
 * faz um dynamic_cast para BaloonAttackParams e, se o cast falhar, escreve uma
 * linha no log e NAO DISPARA NADA. Por isso este bloco e obrigatorio num ataque
 * BaloonAttack e recusado na leitura quando falta - um ataque que nao dispara e
 * pior de achar do que um arquivo recusado.
 *
 * Fica num sub-objeto em vez de solto entre os outros campos para deixar claro
 * que nao valem para as demais estrategias.
 */
struct DescricaoDeBalao {
    /// De onde os baloes sobem ou descem: Down, Left, Right ou Up.
    /// OBRIGATORIO: a propria BaloonAttack recusa side == None.
    /// Convencao da estrategia: "Down" nasce ABAIXO da tela e sobe.
    std::string lado;

    std::optional<bool>  spawnAleatorio;      ///< randomSpawn
    std::optional<bool>  centradoNoJogador;   ///< centerOnPlayer
    std::optional<float> deslocamento;        ///< centerOnPlayerOffset

    /// Posicoes exatas, para o modo NAO aleatorio. Em coordenadas de tela.
    std::vector<Vector2> pontosDeSpawn;
};

/**
 * @brief Um ataque: a geometria do disparo e as regras de cada projetil.
 *
 * OS CAMPOS NUMERICOS SAO OPCIONAIS DE PROPOSITO.
 *
 * Cada um deles corresponde a um campo de AttackParams, e AttackParams JA tem um
 * valor padrao para todos. Se esta struct repetisse esses padroes, existiriam
 * dois lugares dizendo quanto vale o angulo de um leque por omissao - e mudar um
 * sem o outro faria um ataque migrado para dados se comportar diferente do mesmo
 * ataque em C++, silenciosamente.
 *
 * Com optional, a regra fica literal: A PONTE SO ESCREVE O QUE O ARQUIVO DISSE.
 * O campo ausente nao e "zero" nem "um palpite", e o padrao do proprio motor,
 * qualquer que ele seja hoje ou depois.
 */
struct DescricaoDeAtaque {
    std::string estrategia;

    /// Nome da fabrica de projeteis daquele chefe, como ela foi registrada em
    /// ConfigureComponents ("Dados", "Capivara", "Arduino", "Baloes"). Obrigatorio:
    /// sem ele nao ha de onde tirar os projeteis. A camada pura so confere que
    /// nao esta vazio - quais nomes existem e coisa de cada chefe, em C++.
    std::string projetil;

    std::optional<int>   projeteis;       ///< AttackParams::numProjectiles
    std::optional<float> velocidade;      ///< AttackParams::projectileSpeed
    std::optional<float> angulo;          ///< AttackParams::angle, a abertura do leque
    std::optional<float> anguloCentral;   ///< AttackParams::centralAngle, o meio do leque
    std::optional<float> intervalo;       ///< AttackParams::creationSpeed; so a WaveAttack le

    /// Segundos entre dois disparos deste ataque. Este NAO e opcional: e o unico
    /// numero que nao vive em AttackParams, e sim em AddAttackPattern, e nao ha
    /// padrao razoavel para ele.
    ///
    /// Aceita duas formas no arquivo. A primeira e um numero. A segunda deriva o
    /// valor do ritmo da investida escrita EM LINHA neste mesmo ataque:
    ///
    ///     "cooldown": { "doRitmo": "total", "dividirPor": 7 }
    ///     "cooldown": { "doRitmo": "ciclo", "mais": 0.5 }
    ///
    /// A segunda forma existe para nao perder uma propriedade que o codigo C++
    /// tinha: um unico ritmo alimentava a Motion, o Modifier E o cooldown, entao
    /// mexer na pausa movia as miradas e o cooldown JUNTO. Com um numero
    /// literal, mexer no ritmo e esquecer o cooldown deixa varios cacadores na
    /// tela ao mesmo tempo, sem erro nenhum aparecer.
    ///
    /// Depois da leitura este campo ja esta resolvido: quem usa DescricaoDeAtaque
    /// sempre ve um numero.
    float cooldown = 1.0f;

    /// Regras do projetil. Duas formas, mutuamente exclusivas:
    /// - regrasNome: aponta para um conjunto de Assets/Attacks/regras.json
    /// - regras: escritas em linha, aqui mesmo
    std::string regrasNome;
    std::vector<Regra> regras;
    bool temRegrasEmLinha = false;

    /// Presente somente nos ataques BaloonAttack.
    std::optional<DescricaoDeBalao> balao;
};

/// @brief O lado nomeado existe? (Down, Left, Right, Up - nunca None)
bool LadoDeBalaoExiste(const std::string& nome);

/**
 * @brief Como o chefe se move durante a fase.
 *
 * 'a' e 'b' sao os dois parametros do construtor, na ordem em que ele os recebe.
 * Ficam genericos de proposito: cada estrategia da um sentido a eles, e a tabela
 * esta documentada em FasesDeAtaqueArquivo.cpp, onde a instanciacao acontece.
 */
struct DescricaoDeMovimento {
    std::string tipo = "GoToCenter";
    float a = 0.0f;
    float b = 0.0f;
};

/**
 * @brief Uma fase: quanto dura, o que vem depois, como o chefe anda e o que ele
 * dispara.
 *
 * 'nome' precisa ser um dos nomes que a maquina de estados reconhece
 * (StateOne, StateTwo, StateThree, StateFinal). 'proximo' vazio significa que a
 * batalha e decidida ao fim desta fase.
 */
struct DescricaoDeFase {
    std::string nome;
    float duracao = 17.0f;
    std::string proximo;
    DescricaoDeMovimento movimento;
    std::vector<DescricaoDeAtaque> ataques;
};

/**
 * @brief Conjuntos de fases lidos de um arquivo, um por chefe.
 */
struct FasesLidas {
    std::map<std::string, std::vector<DescricaoDeFase>> conjuntos;
    std::vector<std::string> problemas;
};

/// @brief A estrategia de ataque nomeada existe?
bool EstrategiaExiste(const std::string& nome);

/// @brief A estrategia de movimento nomeada existe?
bool MovimentoExiste(const std::string& nome);

/// @brief O nome de fase e um dos que a maquina de estados procura?
bool NomeDeFaseExiste(const std::string& nome);

/**
 * @brief Le fases a partir do TEXTO de um arquivo JSON. Funcao PURA.
 *
 * Formato:
 * {
 *   "ricardo": {
 *     "StateOne": {
 *       "duracao": 17, "proximo": "StateTwo",
 *       "movimento": { "tipo": "RandomWander", "a": 4, "b": 100 },
 *       "ataques": [
 *         { "estrategia": "CircleSpreadAttack", "projetil": "Arduino",
 *           "projeteis": 36, "velocidade": 180, "cooldown": 2.4 }
 *       ]
 *     }
 *   }
 * }
 *
 * Um ataque invalido e descartado sem invalidar a fase; uma fase invalida e
 * descartada sem invalidar o conjunto. Cada problema diz qual conjunto, qual
 * fase e qual ataque.
 */
FasesLidas LerFases(const std::string& textoJson);

/**
 * @brief Confere se as transicoes de um conjunto fecham. Funcao PURA.
 *
 * Existe por causa de uma armadilha real: os nomes de estado sao procurados por
 * TEXTO pela maquina de estados. Um 'proximo' com erro de digitacao, ou a
 * ausencia de StateFinal, fazem a FSM registrar um erro no log e o chefe
 * congelar - sem travar o jogo e sem avisar em tela. Passar a descrever as
 * fases num arquivo multiplicaria essa chance, entao a validacao vem junto.
 *
 * Devolve uma frase por problema; lista vazia significa que o conjunto fecha.
 */
std::vector<std::string> ValidarTransicoes(const std::vector<DescricaoDeFase>& fases,
                                           const std::string& nomeDoConjunto);
