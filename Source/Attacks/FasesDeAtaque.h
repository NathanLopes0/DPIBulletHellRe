//
// Fases de ataque de um chefe, lidas de texto JSON.
//

#pragma once

#include "../Regras/Dificuldade.h"

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "Fase.h"
#include "RegrasDeAtaque.h"

/// Nao abre arquivo, nao escreve log, nao conhece Boss, FSMComponent, AttackParams nem SDL.
/// Transforma texto em descricoes e aponta os problemas.

/// Os campos que so a BaloonAttack usa.
struct DescricaoDeBalao {
    /// De onde os baloes sobem ou descem: Down, Left, Right ou Up. Obrigatorio: a propria
    /// BaloonAttack recusa side == None.
    std::string lado;

    std::optional<bool>  spawnAleatorio;      ///< randomSpawn
    std::optional<bool>  centradoNoJogador;   ///< centerOnPlayer
    std::optional<float> deslocamento;        ///< centerOnPlayerOffset

    /// Posicoes exatas, para o modo nao aleatorio. Em coordenadas de tela.
    std::vector<Vector2> pontosDeSpawn;
};

/// Os campos que so a ConsultaAttack usa. Mesmo contrato do bloco de balao: a ConsultaAttack faz um
/// dynamic_cast e, sem estes campos, nao dispara nada.
struct DescricaoDeConsulta {

    /// "Linha" ou "Coluna". Obrigatorio - nao ha padrao razoavel: varrer o eixo errado e um ataque
    /// que funciona e esta errado, que e pior de achar.
    std::string eixo;

    std::optional<int>   linhas;    ///< Tabela::Forma::linhas
    std::optional<int>   colunas;   ///< Tabela::Forma::colunas

    /// Qual faixa varrer, quando a fase quer uma fixa.
    std::optional<int>   indice;

    /// Segundos de telegrafo.
    std::optional<float> aviso;

    std::optional<bool>  invertido;  ///< entra pelo outro lado
};

/// Os campos que so a CaixaAttack usa.
struct DescricaoDeCaixa {
    std::optional<float> proporcao;           ///< largura / altura
    std::optional<float> meiaAlturaInicial;   ///< em pixels
};

/// Os campos que so a SprintAttack usa. Opcional: todo campo tem padrao em SprintAttackParams.
struct DescricaoDeSprint {
    std::optional<float> revisao;   ///< segundos do disparo ate o que sobrou cair
    std::string historia;           ///< a animacao das historias
    std::string cursor;             ///< a do cursor apontando
    std::string clique;             ///< a do cursor clicando
};

/// Os campos que so a TokenAttack usa. Opcional: todo campo tem padrao em TokenAttackParams.
struct DescricaoDeTokens {
    std::optional<float> temperatura;   ///< a do sorteio; zero = sempre o mais provavel
    std::optional<int> candidatos;      ///< quantos angulos candidatos
    std::optional<float> abertura;      ///< o leque deles, em graus
    std::vector<std::string> frase;     ///< as animacoes dos tokens, em ordem
};

/// Os campos que so a PalavraAttack usa. Opcional: todo campo tem padrao em PalavraAttackParams.
struct DescricaoDePalavra {
    std::optional<std::uint32_t> valor;   ///< os bits, escritos com 0 e 1
    std::optional<float> altura;          ///< o y da fileira; ausente = o do professor
    std::string zero;                     ///< a animacao do zero
    std::string um;                       ///< a do um
    bool todosAtravessam = false;         ///< o um tambem e enfeite (as portas)
};

/// Os campos que so a PontosAttack usa. Obrigatorio nela: sem posto, ela nao teria de onde atirar.
struct DescricaoDePontos {
    std::string posto;      ///< de onde se atira (PostoExiste)
    bool mirar = false;     ///< cada ponto mira o aluno
    bool prever = false;    ///< ...onde ele vai estar (so vale mirando)
};

/// Um ataque: a geometria do disparo e as regras de cada projetil. Os campos numericos sao
/// opcionais.
struct DescricaoDoDificil {
    std::optional<int>   projeteis;
    std::optional<float> velocidade;
    std::optional<float> cooldown;
};

struct DescricaoDeAtaque {
    std::string estrategia;

    /// Nome da fabrica de projeteis daquele chefe, como ela foi registrada em ConfigureComponents
    /// ("Dados", "Capivara", "Arduino", "Baloes").
    std::string projetil;

    std::optional<int>   projeteis;       ///< AttackParams::numProjectiles
    std::optional<float> velocidade;      ///< AttackParams::projectileSpeed
    std::optional<float> angulo;          ///< AttackParams::angle, a abertura do leque
    std::optional<float> anguloCentral;   ///< AttackParams::centralAngle, o meio do leque
    std::optional<float> intervalo;       ///< AttackParams::creationSpeed; so a WaveAttack le

    /// Segundos entre dois disparos deste ataque.
    float cooldown = 1.0f;

    /// Regras do projetil.
    std::string regrasNome;
    std::vector<Regra> regras;
    bool temRegrasEmLinha = false;

    /// Presente somente nos ataques BaloonAttack.
    std::optional<DescricaoDeBalao> balao;

    /// Presente somente nos ataques ConsultaAttack.
    std::optional<DescricaoDeConsulta> consulta;

    /// Presente so nos ataques CaixaAttack que escreveram o bloco.
    std::optional<DescricaoDeCaixa> caixa;

    /// Presente so nos ataques SprintAttack que escreveram o bloco.
    std::optional<DescricaoDeSprint> sprint;

    /// Presente em todo ataque PontosAttack, e so neles.
    std::optional<DescricaoDePontos> pontos;

    /// Presente so nos ataques TokenAttack que escreveram o bloco.
    std::optional<DescricaoDeTokens> tokens;

    /// Presente so nos ataques PalavraAttack que escreveram o bloco.
    std::optional<DescricaoDePalavra> palavra;

    /// Em qual etapa da fase este ataque dispara. Vazio = a fase inteira.
    std::string etapa;

    /// O gatilho que o dispara (GatilhoExiste). Vazio = o relogio, pelo cooldown.
    std::string gatilho;

    /// O degrau do Dificil, se o arquivo escreveu um.
    std::optional<DescricaoDoDificil> dificil;
};

/// O ataque no modo: no Dificil, com o que o bloco "dificil" troca; no Normal (ou sem bloco),
/// igual.
DescricaoDeAtaque NoNivel(const DescricaoDeAtaque& a, Dificuldade::Nivel n);

/// O lado nomeado existe? (Down, Left, Right, Up - nunca None)
bool LadoDeBalaoExiste(const std::string& nome);

/// O eixo nomeado existe? (Linha ou Coluna)
bool EixoDeConsultaExiste(const std::string& nome);

/// O gatilho nomeado existe? Um ataque com "gatilho" nao dispara pelo relogio: dispara quando o
/// chefe puxa o gatilho (Boss::Disparar).
bool GatilhoExiste(const std::string& nome);

/// Todos os gatilhos, na ordem em que entraram.
const std::vector<std::string>& NomesDosGatilhos();

/// Como o chefe se move durante a fase. 'a' e 'b' sao os dois parametros do construtor, na ordem em
/// que ele os recebe.
struct DescricaoDeMovimento {
    std::string tipo = "GoToCenter";
    float a = 0.0f;
    float b = 0.0f;

    /// O bloco "dificil": o 'a' e o 'b' do modo Dificil, quando ele os troca.
    std::optional<float> aNoDificil;
    std::optional<float> bNoDificil;
};

/// O movimento no modo: no Dificil, com o que o bloco "dificil" troca; no Normal (ou sem bloco),
/// igual.
DescricaoDeMovimento NoNivel(const DescricaoDeMovimento& m, Dificuldade::Nivel n);

/// Uma fase: quanto dura, o que vem depois, como o chefe anda e o que ele dispara.
struct DescricaoDeClone {
    bool existe = false;
    std::string ataquesDe;
};

/// O Programa de Intercambio da fase: os alunos trocam de lugar. A cada 'aCada' segundos, com o
/// aviso aceso 'aviso' segundos antes.
struct DescricaoDeIntercambio {
    float aCada = 0.0f;
    float aviso = 0.0f;
};

/// Uma fase: quanto dura, o que vem depois, como o chefe anda e o que ele dispara.
struct DescricaoDeEtapa {
    std::string nome;
    float duracao = 0.0f;
    std::string anuncio;   ///< o texto do placar; vazio = o proprio nome
};

/// O trecho [de, ate) da fase em que a etapa nomeada acontece, contado do comeco da fase. Devolve
/// false se nao houver etapa com esse nome.
bool JanelaDaEtapa(const std::vector<DescricaoDeEtapa>& etapas, const std::string& nome,
                   float& de, float& ate);

/// A etapa que vale no instante 't' da fase: o indice, ou -1 depois da ultima (ou sem etapas).
/// Antes de zero vale a primeira.
int EtapaNoTempo(const std::vector<DescricaoDeEtapa>& etapas, float t);

/// Quanto o primeiro disparo de um ataque espera depois que a janela dele abre.
constexpr float kPrimeiroDisparoDaJanela = 0.25f;

/// Quanto o primeiro disparo de cada ataque espera na primeira vez que a fase dele roda.
constexpr float kPrimeiroDisparoDaFase = 1.0f;

/// Quando sai o primeiro disparo de um ataque cuja janela abre 'de' segundos depois do comeco da
/// fase, na primeira vez que ela roda.
float PrimeiroDisparoNaJanela(float de);

struct DescricaoDeFase {
    std::string nome;
    float duracao = Fase::kDuracao;
    std::string proximo;

    /// As etapas da fase, em ordem. Vazio = a fase e um trecho so, como sempre.
    std::vector<DescricaoDeEtapa> etapas;

    /// O que o placar da batalha mostra enquanto esta fase dura - a linguagem de que cada rajada e
    /// uma palavra, no caso do Vladimir.
    std::string anuncio;

    /// O que o professor diz no balao de fala, na ordem; quando cada uma aparece e o chefe que
    /// decide. Vazio = ele nao fala nesta fase.
    std::vector<std::string> falas;

    DescricaoDeMovimento movimento;
    std::vector<DescricaoDeAtaque> ataques;
    std::string ataquesDe;
    DescricaoDeClone clone;

    /// Presente so nas fases com Intercambio.
    std::optional<DescricaoDeIntercambio> intercambio;
};

/// As fases de um chefe, lidas do arquivo dele.
struct FasesLidas {
    std::vector<DescricaoDeFase> fases;
    std::vector<std::string> problemas;
};

/// A estrategia de ataque nomeada existe?
bool EstrategiaExiste(const std::string& nome);

/// A estrategia de movimento nomeada existe?
bool MovimentoExiste(const std::string& nome);

/// O nome de fase e um dos que a maquina de estados procura?
bool NomeDeFaseExiste(const std::string& nome);

/// Le fases a partir do texto de um arquivo JSON. Funcao pura.
FasesLidas LerFases(const std::string& textoJson);

/// Confere se as transicoes de um conjunto fecham. Funcao pura.
std::vector<std::string> ValidarTransicoes(const std::vector<DescricaoDeFase>& fases,
                                           const std::string& nomeDoConjunto);
