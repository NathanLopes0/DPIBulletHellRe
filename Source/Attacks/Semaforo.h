//
// A fase 1 do Vitor: os semaforos.
//

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Fase.h"

/**
 * CAMADA PURA.
 *
 * A FASE 1 DO VITOR E A SINCRONIZACAO: espera ocupada e semaforos, o comeco
 * da ementa da INF 310.
 *
 * Cada THREAD e uma coluna: o nome dela no alto (T1, T2...) e as instrucoes
 * descendo, uma atras da outra. No meio do campo, de borda a borda, fica a
 * SECAO CRITICA - a faixa do saldo++. Antes de entrar nela toda thread faz
 * P(s):
 *
 *   - se o semaforo tem permissao, ela entra, e a coluna atravessa a faixa e
 *     desce sobre o aluno;
 *   - se nao tem, ela BLOQUEIA: as instrucoes param na borda da faixa e se
 *     empilham, a fila do P(s). A thread bloqueada nao anda - quando a pilha
 *     enche, ela para de mandar instrucao;
 *   - quem sai faz V(s), e o semaforo acorda a primeira da fila: a pilha
 *     inteira desce de uma vez, numa rajada em leque.
 *
 * O PLACAR MOSTRA O VALOR DE s E A FILA, na ordem: quem le a fila sabe de que
 * coluna vem a proxima rajada. O professor fica ao lado da coluna que ele vai
 * acordar.
 *
 * Quatro rodadas, uma por jeito de sincronizar:
 *
 *   mutex            s = 1: uma de cada vez, na ordem de chegada
 *   semaforo         s = 2: duas de cada vez
 *   espera ocupada   while (test_and_set(&trava)); - as bloqueadas GIRAM na
 *                    porta, gastando processador, e quem entra nao e a
 *                    primeira da fila, e quem ganhar a corrida; ao entrar, o
 *                    giro estoura num anel
 *   corrida          sem trava nenhuma: todas entram juntas, as instrucoes se
 *                    intercalam (cada uma sai torta da faixa), e o placar
 *                    mostra o saldo errado
 *
 * AS CORRENTES CONTINUAM: o professor as atira, e o aluno acorrentado fica
 * lento. E e pior: ele esta bloqueado em P(s), e a thread em cima dele FURA A
 * FILA - e a proxima a entrar (Furar).
 *
 * Aqui ficam as rodadas, as colunas, o escalonador, a pilha, o giro, a
 * corrida, os relogios e os textos; quem cria, empilha e solta as instrucoes
 * e o Vitor. Coberto por test_semaforo.cpp.
 */
namespace Semaforo {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDosSemaforos = "StateOne";
    constexpr const char* kGatilhoDaInstrucao = "instrucao";

    // ---- AS RODADAS ---------------------------------------------------------

    enum class Tipo { Mutex, Contador, EsperaOcupada, Corrida };

    struct Rodada {
        Tipo tipo = Tipo::Mutex;
        int permissoes = 1;          ///< o valor inicial de s; na corrida, ninguem espera
    };

    constexpr int kRodadas = 4;
    Rodada RodadaDaVez(int k);

    // ---- AS THREADS ---------------------------------------------------------

    /// As colunas: uma EM CIMA DO ALUNO - parado, ele nao fica entre duas -,
    /// as outras a cada kEntreThreads, dentro do campo, ate kMaisThreads, as
    /// mais perto dele. Da esquerda para a direita: T1, T2...
    std::vector<float> Colunas(float xDoAluno, float largura);
    constexpr float kEntreThreads = 220.0f;
    constexpr float kMargem = 70.0f;
    constexpr int kMaisThreads = 5;

    /// @brief A coluna mais perto de x (-1 se nao ha coluna).
    int MaisPerto(const std::vector<float>& colunas, float x);

    /// "T1", "T2"...: o nome da thread de indice i.
    std::string Nome(int i);

    // ---- O ESCALONADOR: o semaforo -------------------------------------------

    /// O estado do semaforo numa rodada: quem espera, na ordem, quem esta na
    /// secao critica (e desde quando) e quem ja passou.
    struct Escalonador {
        Rodada rodada;
        std::vector<int> fila;
        std::vector<int> dentro;
        std::vector<float> entrouEm;
        std::vector<int> passaram;
        std::uint32_t sorteio = 0;
        float agora = 0.0f;          ///< o 'u' do ultimo Avancar
    };

    /**
     * @brief Comeca a rodada: a ORDEM DE CHEGADA na porta e sorteada, com a
     * thread do aluno (doAluno) entre as tres primeiras - quem fica parado em
     * cima de uma coluna nao espera a rodada inteira por ela.
     */
    Escalonador Comecar(const Rodada& r, int threads, int doAluno, std::uint32_t sorteio);

    /// O que aconteceu na porta.
    struct Evento {
        bool entra = true;           ///< true: P(s) passou; false: V(s)
        int thread = 0;
    };

    /**
     * @brief Leva o semaforo ate 'u' segundos desde o comeco da rodada.
     *
     * Ninguem entra antes de kAbre (as primeiras instrucoes ainda estao
     * chegando). Dali, as primeiras 'permissoes' da fila entram; cada uma fica
     * NaSecao(rodada) e sai, e cada saida acorda a seguinte - a primeira da
     * fila, ou, na espera ocupada, uma sorteada entre as que giram. Na corrida
     * todas entram juntas em kAbre, e ninguem sai.
     */
    std::vector<Evento> Avancar(Escalonador& e, float u);

    /**
     * @brief A thread t FURA A FILA: vai para a frente das que esperam. E o
     * que acontece com a coluna em cima do aluno acorrentado - ele esta
     * bloqueado em P(s), e e a vez dele. Devolve false se t nao esta
     * esperando (ja entrou, ja passou, ou ja e a primeira).
     */
    bool Furar(Escalonador& e, int t);

    /// @brief Quanto vale s agora: as permissoes menos quem esta dentro.
    int Valor(const Escalonador& e);

    bool Esperando(const Escalonador& e, int t);
    bool Dentro(const Escalonador& e, int t);
    bool Passou(const Escalonador& e, int t);

    /// Quem entra a seguir: a primeira da fila (-1 se ninguem espera). Na
    /// espera ocupada nao se sabe - e o professor nao vai ate nenhuma.
    int Proxima(const Escalonador& e);

    // ---- AS INSTRUCOES ------------------------------------------------------

    /// O alto das colunas, de onde as instrucoes nascem, e a faixa da secao
    /// critica. Abaixo do placar (Battle: 12 + 46) e bem acima do aluno.
    constexpr float kAlturaDosNomes = 84.0f;
    constexpr float kNasce = 112.0f;
    constexpr float kTopoDaSecao = 292.0f;
    constexpr float kAlturaDaSecao = 48.0f;

    /// A velocidade e a do ataque em fases.json (um teste confere).
    constexpr float kVelocidadeDaInstrucao = 200.0f;
    constexpr float kEntreInstrucoes = 0.3f;
    constexpr float kComecaAMandar = 0.1f;
    constexpr int kInstrucoesPorThread = 8;

    /// A PILHA do P(s): ate kMaiorPilha instrucoes paradas, uma em cima da
    /// outra, a partir da borda da faixa. Cheia, a thread para de mandar.
    constexpr int kMaiorPilha = 6;
    constexpr float kEntreEmpilhadas = 28.0f;
    float AlturaNaPilha(int i);

    /// A RAJADA de quem entra: a pilha desce em leque, cada instrucao num
    /// angulo (graus; 90 e reto para baixo), alternando os lados.
    constexpr float kVelocidadeDaRajada = 320.0f;
    float AnguloNaRajada(int i);
    constexpr float kAberturaDaRajada = 7.0f;

    /// A ESPERA OCUPADA: as bloqueadas giram num circulo acima da faixa, e ao
    /// entrar saem pelo raio, num anel.
    struct Ponto { float x = 0.0f; float y = 0.0f; };
    Ponto NoGiro(int i, int n, float t);   ///< o deslocamento a partir do centro do giro
    constexpr float kCentroDoGiro = kTopoDaSecao - 46.0f;
    constexpr float kRaioDoGiro = 30.0f;
    constexpr float kVoltasPorSegundo = 1.2f;
    constexpr float kVelocidadeDoAnel = 260.0f;

    /// A CORRIDA: cada instrucao sai da faixa torta, para um lado sorteado - as
    /// threads se intercalando sem trava.
    float DesvioNaCorrida(std::uint32_t sorteio);
    constexpr float kMaiorDesvio = 14.0f;

    // ---- O RELOGIO ----------------------------------------------------------

    /// Uma rodada a cada kPorRodada, a primeira kPrimeiro depois de a fase
    /// abrir. A porta abre kAbre depois de a rodada comecar.
    constexpr float kPrimeiro = 0.5f;
    constexpr float kPorRodada = (Fase::kDuracao - kPrimeiro) / static_cast<float>(kRodadas);
    constexpr float kAbre = 1.3f;

    /// Quanto cada thread fica na secao critica.
    float NaSecao(const Rodada& r);

    // ---- OS TEXTOS ----------------------------------------------------------

    /// O canto da faixa: o codigo da secao critica.
    std::string CodigoDaSecao(Tipo t);

    /// O placar: o valor de s e a fila ("s = 0 · fila: T4 T1 T5"); na espera
    /// ocupada, quem gira; na corrida, o saldo. Com o aluno acorrentado, ele
    /// diz que o aluno esta em P(s).
    std::string Placar(const Escalonador& e, bool alunoBloqueado);

    /// O SALDO DA CORRIDA: cada thread le o saldo, soma um e escreve. Todas
    /// leram o mesmo valor, e o saldo termina em 1 em vez de 'threads' - a
    /// atualizacao perdida. O placar o mostra kSaldoAparece depois de elas
    /// entrarem, quando as instrucoes ja escreveram.
    int SaldoDaCorrida(int threads);
    constexpr float kSaldoAparece = 1.0f;
}
