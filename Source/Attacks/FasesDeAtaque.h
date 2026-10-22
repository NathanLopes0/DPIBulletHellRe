//
// Fases de ataque de um chefe, lidas de texto JSON.
//

#pragma once

#include <cstdint>
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
 * @brief Os campos que SO a ConsultaAttack usa.
 *
 * Mesmo contrato do bloco de balao: a ConsultaAttack faz um dynamic_cast e, sem
 * estes campos, nao dispara nada. Por isso o bloco e obrigatorio num ataque
 * ConsultaAttack e recusado quando falta.
 */
struct DescricaoDeConsulta {

    /// "Linha" ou "Coluna". OBRIGATORIO - nao ha padrao razoavel: varrer o eixo
    /// errado e um ataque que funciona e esta errado, que e pior de achar.
    std::string eixo;

    std::optional<int>   linhas;    ///< Tabela::Forma::linhas
    std::optional<int>   colunas;   ///< Tabela::Forma::colunas

    /// Qual faixa varrer, quando a fase quer uma fixa. Omitido, quem decide e o
    /// chefe em CustomizeAttackParams - que e o caso das fases que perseguem o
    /// jogador.
    std::optional<int>   indice;

    /// Segundos de telegrafo. Ver ConsultaAttackParams::aviso: e a diferenca
    /// entre dificil e injusto, e por isso a leitura recusa valores negativos.
    std::optional<float> aviso;

    std::optional<bool>  invertido;  ///< entra pelo outro lado
};

/**
 * @brief Os campos que SO a CaixaAttack usa.
 *
 * Diferente dos blocos de balao e de consulta, este e OPCIONAL: os dois campos
 * tem um padrao razoavel (CaixaAttackParams), e uma caixa sem bloco e uma caixa
 * na proporcao de uma janela deitada.
 */
struct DescricaoDeCaixa {
    std::optional<float> proporcao;           ///< largura / altura
    std::optional<float> meiaAlturaInicial;   ///< em pixels
};

/**
 * @brief Os campos que SO a CapsulaAttack usa. Opcional, como o da caixa: todo
 * campo tem padrao em CapsulaAttackParams.
 */
struct DescricaoDeCapsula {
    std::optional<float> abre;         ///< segundos fechada
    std::optional<float> velocidade;   ///< o modulo das letras depois de aberta
    std::string escreve;               ///< a animacao delas depois de aberta
};

/**
 * @brief Os campos que SO a SerpenteAttack usa. Opcional: todo campo tem padrao
 * em SerpenteAttackParams.
 */
struct DescricaoDeSerpente {
    std::optional<float> espaco;    ///< pixels de caminho entre dois gomos
    std::optional<float> giro;      ///< radianos por segundo
    std::optional<float> duracao;   ///< segundos perseguindo
    std::string cabeca;             ///< a animacao da cabeca
    std::string corpo;              ///< a animacao dos gomos
};

/**
 * @brief Os campos que SO a SprintAttack usa. Opcional: todo campo tem padrao
 * em SprintAttackParams.
 */
struct DescricaoDeSprint {
    std::optional<float> revisao;   ///< segundos do disparo ate o que sobrou cair
    std::string historia;           ///< a animacao das historias
    std::string cursor;             ///< a do cursor apontando
    std::string clique;             ///< a do cursor clicando
};

/**
 * @brief Os campos que SO a TokenAttack usa. Opcional: todo campo tem padrao
 * em TokenAttackParams.
 */
struct DescricaoDeTokens {
    std::optional<float> temperatura;   ///< a do sorteio; zero = sempre o mais provavel
    std::optional<int> candidatos;      ///< quantos angulos candidatos
    std::optional<float> abertura;      ///< o leque deles, em graus
    std::vector<std::string> frase;     ///< as animacoes dos tokens, em ordem
};

/**
 * @brief Os campos que SO a PalavraAttack usa. Opcional: todo campo tem padrao
 * em PalavraAttackParams.
 */
struct DescricaoDePalavra {
    std::optional<std::uint32_t> valor;   ///< os bits, escritos com 0 e 1
    std::optional<float> altura;          ///< o y da fileira; ausente = o do professor
    std::string zero;                     ///< a animacao do zero
    std::string um;                       ///< a do um
    bool todosAtravessam = false;         ///< o um tambem e enfeite (as portas)
};

/**
 * @brief Os campos que SO a PontosAttack usa. OBRIGATORIO nela: sem posto, ela
 * nao teria de onde atirar.
 */
struct DescricaoDePontos {
    std::string posto;      ///< de onde se atira (PostoExiste)
    bool mirar = false;     ///< cada ponto mira o aluno
    bool prever = false;    ///< ...onde ele vai estar (so vale mirando)
};

/**
 * @brief Os campos que SO a FlexAttack usa.
 *
 * "de" e "para" sao OBRIGATORIOS e tem de ser nomes de justify-content do CSS
 * de verdade (Flex::PeloNome): quem estudou a materia vai ler o placar, e um
 * nome inventado ensinaria CSS errado. O resto tem padrao em FlexAttackParams.
 */
struct DescricaoDeFlex {
    std::string de;
    std::string para;
    std::optional<float> aos;
    std::optional<float> em;
    std::optional<float> item;
    std::optional<float> largura;
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

    /// Presente somente nos ataques ConsultaAttack.
    std::optional<DescricaoDeConsulta> consulta;

    /// Presente so nos ataques CaixaAttack que escreveram o bloco.
    std::optional<DescricaoDeCaixa> caixa;

    /// Presente somente nos ataques FlexAttack.
    std::optional<DescricaoDeFlex> flex;

    /// Presente so nos ataques CapsulaAttack que escreveram o bloco.
    std::optional<DescricaoDeCapsula> capsula;

    /// Presente so nos ataques SerpenteAttack que escreveram o bloco.
    std::optional<DescricaoDeSerpente> serpente;

    /// Presente so nos ataques SprintAttack que escreveram o bloco.
    std::optional<DescricaoDeSprint> sprint;

    /// Presente em todo ataque PontosAttack, e so neles.
    std::optional<DescricaoDePontos> pontos;

    /// Presente so nos ataques TokenAttack que escreveram o bloco.
    std::optional<DescricaoDeTokens> tokens;

    /// Presente so nos ataques PalavraAttack que escreveram o bloco.
    std::optional<DescricaoDePalavra> palavra;

    /// Em qual etapa da fase este ataque dispara. Vazio = a fase inteira. Fora
    /// da etapa ele nao conta o cooldown nem dispara.
    std::string etapa;

    /// O gatilho que o dispara (GatilhoExiste). Vazio = o relogio, pelo
    /// cooldown. Com gatilho, o cooldown e ignorado.
    std::string gatilho;
};

/// @brief O lado nomeado existe? (Down, Left, Right, Up - nunca None)
bool LadoDeBalaoExiste(const std::string& nome);

/// @brief O eixo nomeado existe? (Linha ou Coluna)
bool EixoDeConsultaExiste(const std::string& nome);

/// @brief O gatilho nomeado existe? Um ataque com "gatilho" nao dispara pelo
/// relogio: dispara quando o chefe puxa o gatilho (Boss::Disparar). So existe
/// o gatilho que algum chefe puxa: "notificacao" e "consulta" sao o sujeito
/// do padrao Observer da Maria Lucia (Observer.h), "agir" e o ciclo dos
/// agentes do Julio (ReAct.h), "nuvens" e "chover" as nuvens do Vitor
/// (Nuvem.h), "portas" a fileira de portas logicas do Ricardo (Logica.h),
/// "contador" o leque do clock dele (Relogio.h), e "estagios", "buscar",
/// "lw", "sw", "add" e "addi" o pipeline dele (Pipeline.h) - os quatro
/// ultimos, o ataque de cada instrucao no seu estagio.
bool GatilhoExiste(const std::string& nome);

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
/**
 * @brief O CORPO EXTRA de uma fase: o chefe se divide em dois.
 *
 * Existe para a fase 3 do Vitor, em que a materia e "varias coisas ao mesmo
 * tempo" e seria estranho a batalha fazer uma de cada vez. Enquanto a fase
 * dura, ha DOIS corpos em campo: o original e este. Os dois disparam, e
 * qualquer um dos dois toma dano.
 *
 * 'ataquesDe' nomeia de qual fase o clone tira os ataques e o movimento - e
 * nao uma lista propria. Escrever a lista aqui faria a fase 3 repetir, palavra
 * por palavra, o que a fase 2 ja diz, e duas copias divergem no dia em que
 * alguem ajusta uma.
 */
struct DescricaoDeClone {
    bool existe = false;
    std::string ataquesDe;
};

/**
 * @brief O Programa de Intercambio da fase: os alunos trocam de lugar.
 *
 * A cada 'aCada' segundos, com o aviso aceso 'aviso' segundos antes. Ver
 * Intercambio.h para o que "trocar de lugar" quer dizer em dupla e sozinho.
 */
struct DescricaoDeIntercambio {
    float aCada = 0.0f;
    float aviso = 0.0f;
};

/**
 * @brief Uma fase: quanto dura, o que vem depois, como o chefe anda e o que ele
 * dispara.
 *
 * 'nome' precisa ser um dos nomes que a maquina de estados reconhece
 * (StateOne, StateTwo, StateThree, StateFinal). 'proximo' vazio significa que a
 * batalha e decidida ao fim desta fase.
 *
 * 'ataquesDe', quando preenchido, diz que esta fase usa os ataques de OUTRA em
 * vez de declarar os seus - ver DescricaoDeClone. Uma fase nao pode ter os
 * dois: 'ataques' e 'ataquesDe' juntos seriam duas respostas para "o que ele
 * dispara aqui".
 */
/**
 * @brief UMA ETAPA de uma fase: um trecho do tempo dela, com nome.
 *
 * Existe para a fase que e um PROCESSO em ordem - a cascata da Maria Lucia, em
 * que Requisitos acaba antes de Projeto comecar. As etapas se seguem na ordem do
 * arquivo, cada uma 'duracao' segundos; cada ataque diz em qual etapa dispara
 * ("etapa" no ataque), e o placar mostra o 'anuncio' da etapa corrente.
 */
struct DescricaoDeEtapa {
    std::string nome;
    float duracao = 0.0f;
    std::string anuncio;   ///< o texto do placar; vazio = o proprio nome
};

/// @brief O trecho [de, ate) da fase em que a etapa nomeada acontece, contado
/// do comeco da fase. Devolve false se nao houver etapa com esse nome.
bool JanelaDaEtapa(const std::vector<DescricaoDeEtapa>& etapas, const std::string& nome,
                   float& de, float& ate);

/// @brief A etapa que vale no instante 't' da fase: o indice, ou -1 depois da
/// ultima (ou sem etapas). Antes de zero vale a primeira.
int EtapaNoTempo(const std::vector<DescricaoDeEtapa>& etapas, float t);

/// Quanto o primeiro disparo de um ataque espera depois que a janela dele
/// abre. Pouco, mas nao zero: a etapa anterior acabou de parar, e os dois
/// ataques nascendo no mesmo quadro se leriam como um so.
constexpr float kPrimeiroDisparoDaJanela = 0.25f;

/// Quanto o primeiro disparo de cada ataque espera na primeira vez que a FASE
/// dele roda. E maior que o da janela, e vale tambem para o ataque cuja etapa
/// abre a fase: uma etapa que comeca no zero dispara aos 1 s, e as outras
/// 0,25 s depois de abrir.
constexpr float kPrimeiroDisparoDaFase = 1.0f;

/// @brief Quando sai o primeiro disparo de um ataque cuja janela abre 'de'
/// segundos depois do comeco da fase, na primeira vez que ela roda. E a conta
/// que o Boss faz quadro a quadro (Boss::OnUpdate), escrita uma vez para os
/// dados poderem confere-la: quem precisa que todas as etapas disparem no
/// mesmo ponto - as sprints da Maria Lucia - comeca a primeira depois do zero.
float PrimeiroDisparoNaJanela(float de);

struct DescricaoDeFase {
    std::string nome;
    float duracao = 17.0f;
    std::string proximo;

    /// As etapas da fase, em ordem. Vazio = a fase e um trecho so, como sempre.
    std::vector<DescricaoDeEtapa> etapas;

    /// O que o PLACAR da batalha mostra enquanto esta fase dura - a linguagem
    /// de que cada rajada e uma palavra, no caso do Vladimir. Vazio = a fase nao
    /// anuncia nada e o placar fica apagado. Quem desenha e Battle::AtualizarPlacar.
    std::string anuncio;

    DescricaoDeMovimento movimento;
    std::vector<DescricaoDeAtaque> ataques;
    std::string ataquesDe;
    DescricaoDeClone clone;

    /// Presente so nas fases com Intercambio.
    std::optional<DescricaoDeIntercambio> intercambio;
};

/**
 * @brief AS FASES DE UM CHEFE, lidas do arquivo dele.
 *
 * Era um mapa de conjuntos, com o nome do chefe por fora. Esse nome saiu junto
 * com o arquivo unico: ele passou a estar na PASTA
 * (Assets/Attacks/<chefe>/fases.json), e mante-lo aqui dentro criaria duas
 * versoes da mesma verdade, livres para discordar.
 *
 * NA ORDEM DO ARQUIVO, que nao e a ordem em que as fases acontecem - quem
 * encadeia e o campo "proximo" de cada uma. Ver ValidarTransicoes.
 */
struct FasesLidas {
    std::vector<DescricaoDeFase> fases;
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
 * Formato (o arquivo de UM chefe - a pasta diz qual):
 * {
 *   "StateOne": {
 *     "duracao": 17, "proximo": "StateTwo",
 *     "movimento": { "tipo": "RandomWander", "a": 4, "b": 100 },
 *     "ataques": [
 *       { "estrategia": "CircleSpreadAttack", "projetil": "Arduino",
 *         "projeteis": 36, "velocidade": 180, "cooldown": 2.4 }
 *     ]
 *   }
 * }
 *
 * Um ataque invalido e descartado sem invalidar a fase; uma fase invalida e
 * descartada sem invalidar as outras. Cada problema diz qual fase e qual ataque.
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
