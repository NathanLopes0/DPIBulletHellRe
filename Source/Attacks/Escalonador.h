//
// A fase 1 do Goulart: o escalonador de processos.
//

#pragma once

#include <array>
#include <deque>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "../Regras/Dificuldade.h"
#include "Fase.h"

/// A fase 1 do Goulart e o escalonador Round Robin. O professor e a CPU; os processos sao cartoes
/// que andam entre a fila de prontos (a esquerda do campo), a CPU (embaixo do professor) e a E/S (a
/// direita). O campo e dividido em kFaixas faixas verticais, e cada processo tem a sua:
///   - a cada quantum o primeiro da fila sobe para a CPU e roda - e rodar e chover instrucoes na
///     faixa dele, de cima a baixo. O limitado por CPU chove o quantum inteiro; o limitado por E/S,
///     uma rajada curta, e vai esperar o disco; o de tres threads chove em tres faixas;
///   - acabou o quantum e ainda ha fila, ele perde a CPU (a troca de contexto: a chuva para) e volta
///     para o fim da fila;
///   - a faixa do proximo da fila fica marcada: quem le a fila sabe onde vai chover.
/// O aluno pode matar um processo da fila com tiros (kill -9): a faixa dele nao chove mais. Tudo anda
/// pelo relogio do escalonador, e nao pelo que esta em campo: o Ponto Extra limpa a tela, e a CPU
/// continua escalonando.
namespace Escalonador {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFase = "StateOne";
    constexpr const char* kGatilhoDoProcesso = "processo";
    constexpr const char* kGatilhoDaInstrucao = "instrucao";
    constexpr const char* kGatilhoDoTitulo = "titulo";

    enum class Tipo { Cpu, EntradaSaida, Threads };

    /// O que roda na CPU de um processo, e o quanto o de E/S roda antes de pedir o disco.
    struct Processo {
        int pid = 0;
        Tipo tipo = Tipo::Cpu;
        float falta = 0.0f;      ///< CPU que ainda falta, em segundos
        float rajada = 0.0f;     ///< (E/S) quanto roda antes de pedir E/S
        float naRajada = 0.0f;   ///< quanto ja rodou nesta rajada
        float acordaEm = 0.0f;   ///< (bloqueado) quando a E/S termina
    };

    enum class Evento { Despachou, Preemptou, PediuES, Acordou, Terminou };
    struct Aviso {
        Evento evento;
        int pid;
    };

    /// A troca de contexto: a CPU fica ociosa este tanto entre um processo e o seguinte.
    constexpr float kTrocaDeContexto = 0.18f;
    /// Quanto dura a E/S de quem a pede.
    constexpr float kTempoDeES = 1.5f;

    /// O escalonador Round Robin, com o relogio de fora: Avancar(t) leva o estado ate o instante t
    /// e devolve o que aconteceu no caminho, na ordem.
    class RoundRobin {
    public:
        explicit RoundRobin(float quantum = 1.2f, float troca = kTrocaDeContexto);

        /// Um processo novo vai para o fim da fila de prontos.
        void Chegar(const Processo& p);
        std::vector<Aviso> Avancar(float agora);
        /// kill -9: o processo sai de onde estiver. Devolve se ele existia.
        bool Matar(int pid);

        [[nodiscard]] std::optional<int> Rodando() const { return mRodando; }
        [[nodiscard]] const std::deque<int>& Prontos() const { return mProntos; }
        [[nodiscard]] const std::vector<int>& Bloqueados() const { return mBloqueados; }
        [[nodiscard]] const Processo* Achar(int pid) const;
        [[nodiscard]] int Quantos() const { return static_cast<int>(mProcessos.size()); }
        [[nodiscard]] float Quantum() const { return mQuantum; }
        /// Quanto do quantum do processo que roda ja passou, de 0 a 1.
        [[nodiscard]] float NoQuantum() const;

    private:
        float mQuantum;
        float mTroca;
        float mRelogio = 0.0f;
        float mInicioDoQuantum = 0.0f;
        float mOciosaAte = 0.0f;
        std::map<int, Processo> mProcessos;
        std::deque<int> mProntos;
        std::vector<int> mBloqueados;
        std::optional<int> mRodando;
    };

    /// O tipo do k-esimo fork: os tres, em rodizio.
    Tipo TipoDoFork(int k);

    /// Um processo novo do tipo 'tipo': quanta CPU ele precisa e, se de E/S, de quanto em quanto
    /// pede o disco.
    Processo Novo(int pid, Tipo tipo);

    /// A cara do cartao: "P1" a "P9", pela ordem do pid.
    std::string Cara(int pid);

    /// O placar: "Round Robin, q = 1,2 s  ·  CPU: P3  ·  prontos: P1 P4  ·  E/S: P2".
    std::string Placar(const RoundRobin& rr);

    /// Os titulos das duas filas, que sao as caras do projetil do titulo (a CPU e o professor).
    constexpr std::array<const char*, 2> kTitulos = {"Prontos", "ES"};

    /// O que o Dificil muda.
    struct Degrau {
        float quantum = 1.2f;
        int processos = 5;          ///< quantos vivos, no maximo
        float entreForks = 2.6f;    ///< segundos entre um fork e o seguinte
        float cadencia = 1.0f;      ///< o que multiplica o tempo entre as instrucoes
        float chuva = 1.0f;         ///< o que multiplica a velocidade da chuva
    };
    Degrau DegrauDo(Dificuldade::Nivel n);

    // ---- As faixas ----

    /// O campo em kFaixas faixas verticais; a do processo e a do pid, dando a volta.
    constexpr int kFaixas = 6;
    int FaixaDo(int pid);

    /// As faixas em que o processo chove quando roda: a dele e, no de tres threads, as vizinhas.
    std::vector<int> FaixasQueRodam(const Processo& p);

    /// A faixa 'f' num campo que comeca em 'x' e tem 'largura': onde comeca e quanto mede.
    float InicioDaFaixa(int f, float x, float largura);
    float LarguraDaFaixa(float largura);

    /// A chuva da faixa que roda: uma instrucao a cada kEntreGotas (vezes a cadencia do degrau), em
    /// cada faixa, caindo a kVelocidadeDaChuva.
    constexpr float kEntreGotas = 0.06f;
    constexpr float kVelocidadeDaChuva = 270.0f;
}
