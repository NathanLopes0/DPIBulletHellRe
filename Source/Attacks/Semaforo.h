//
// A fase 1 do Vitor: os semaforos.
//

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "Fase.h"
#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

/// A fase 1 do Vitor e a sincronizacao: espera ocupada e semaforos, o comeco da ementa da INF 310.
namespace Semaforo {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDosSemaforos = "StateOne";
    constexpr const char* kGatilhoDaInstrucao = "instrucao";

    // ---- As rodadas ----

    enum class Tipo { Mutex, Contador, EsperaOcupada, Corrida };

    struct Rodada {
        Tipo tipo = Tipo::Mutex;
        int permissoes = 1;          ///< o valor inicial de s; na corrida, ninguem espera
    };

    constexpr int kRodadas = 4;

    /// A ordem de sempre: mutex, s = 2, espera ocupada, corrida.
    Rodada RodadaDaVez(int k);

    // ---- As threads ----

    /// As colunas: uma em cima do aluno. Da esquerda para a direita: T1, T2...
    constexpr float kEntreThreads = 220.0f;
    std::vector<float> Colunas(float xDoAluno, float largura, float entreThreads = kEntreThreads);
    constexpr float kMargem = 70.0f;
    constexpr int kMaisThreads = 5;

    /// A coluna mais perto de x (-1 se nao ha coluna).
    int MaisPerto(const std::vector<float>& colunas, float x);

    /// "T1", "T2"...: o nome da thread de indice i.
    std::string Nome(int i);

    // ---- O escalonador: o semaforo ----

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

    /// Comeca a rodada: a ordem de chegada na porta e sorteada, com a thread do aluno (doAluno)
    /// entre as tres primeiras.
    Escalonador Comecar(const Rodada& r, int threads, int doAluno, std::uint32_t sorteio);

    /// O que aconteceu na porta.
    struct Evento {
        bool entra = true;           ///< true: P(s) passou; false: V(s)
        int thread = 0;
    };

    /// Leva o semaforo ate 'u' segundos desde o comeco da rodada. Ninguem entra antes de kAbre (as
    /// primeiras instrucoes ainda estao chegando).
    std::vector<Evento> Avancar(Escalonador& e, float u);

    /// A thread t fura a fila: vai para a frente das que esperam.
    bool Furar(Escalonador& e, int t);

    int Valor(const Escalonador& e);

    bool Dentro(const Escalonador& e, int t);
    bool Passou(const Escalonador& e, int t);

    /// Quem entra a seguir: a primeira da fila (-1 se ninguem espera). Na
    /// espera ocupada nao se sabe - e o professor nao vai ate nenhuma.
    int Proxima(const Escalonador& e);

    // ---- As instrucoes ----

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

    /// A pilha do P(s): ate kMaiorPilha instrucoes paradas, uma em cima da outra, a partir da borda
    /// da faixa. Cheia, a thread para de mandar.
    constexpr int kMaiorPilha = 6;
    constexpr float kEntreEmpilhadas = 28.0f;
    float AlturaNaPilha(int i);

    /// A rajada de quem entra: a pilha desce em leque, cada instrucao num angulo (graus; 90 e reto
    /// para baixo), alternando os lados.
    constexpr float kVelocidadeDaRajada = 320.0f;
    constexpr float kAberturaDaRajada = 7.0f;
    float AnguloNaRajada(int i, float abertura = kAberturaDaRajada);

    /// A espera ocupada: as bloqueadas giram num circulo acima da faixa, e ao entrar saem pelo
    /// raio, num anel.
    struct Ponto { float x = 0.0f; float y = 0.0f; };
    Ponto NoGiro(int i, int n, float t);   ///< o deslocamento a partir do centro do giro
    constexpr float kCentroDoGiro = kTopoDaSecao - 46.0f;
    constexpr float kRaioDoGiro = 30.0f;
    constexpr float kVoltasPorSegundo = 1.2f;
    constexpr float kVelocidadeDoAnel = 260.0f;

    /// A corrida: cada instrucao sai da faixa torta, para um lado sorteado - as threads se
    /// intercalando sem trava.
    float DesvioNaCorrida(std::uint32_t sorteio);
    constexpr float kMaiorDesvio = 14.0f;

    // ---- O relogio ----

    /// Uma rodada a cada kPorRodada, a primeira kPrimeiro depois de a fase
    /// abrir. A porta abre kAbre depois de a rodada comecar.
    constexpr float kPrimeiro = 0.5f;
    constexpr float kPorRodada = (Fase::kDuracao - kPrimeiro) / static_cast<float>(kRodadas);
    constexpr float kAbre = 1.3f;

    /// Quanto cada thread fica na secao critica.
    float NaSecao(const Rodada& r);

    // ---- Os textos ----

    /// O canto da faixa: o codigo da secao critica.
    std::string CodigoDaSecao(Tipo t);

    /// O placar: o valor de s e a fila ("s = 0 · fila: T4 T1 T5"); na espera ocupada, quem gira; na
    /// corrida, o saldo.
    std::string Placar(const Escalonador& e, bool alunoBloqueado);

    // ---- O que muda de uma luta para a outra (Variacao.h) ----

    /// A fase desta batalha: a ordem das rodadas e os numeros delas.
    struct Plano {
        std::array<Rodada, kRodadas> rodadas{};
        float entreThreads = kEntreThreads;
    };
    Plano PlanoDeSempre();
    Plano Sortear(Variacao::Sorteio& s);
    constexpr int kMenorContador = 2;
    constexpr int kMaiorContador = 3;
    constexpr float kMenorEntreThreads = 200.0f;
    constexpr float kMaiorEntreThreads = 250.0f;

    // ---- O degrau (Dificuldade.h) ----

    /// O que o Dificil muda: as threads mandam mais instrucoes, mais depressa, e a rajada de quem
    /// entra sai mais rapida e mais aberta.
    struct Degrau {
        float entreInstrucoes = kEntreInstrucoes;
        int instrucoesPorThread = kInstrucoesPorThread;
        float rapidez = 1.0f;          ///< multiplica a da instrucao, da rajada e do anel
        float aberturaDaRajada = kAberturaDaRajada;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);

    /// O saldo da corrida: cada thread le o saldo, soma um e escreve.
    int SaldoDaCorrida(int threads);
    constexpr float kSaldoAparece = 1.0f;
}
