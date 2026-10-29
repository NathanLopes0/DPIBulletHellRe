//
// A fase 3 do Lucas: a arvore de supervisao do Elixir.
//

#pragma once

#include <array>
#include <string>
#include <vector>

#include "../../Gameplay/Dificuldade.h"
#include "../../Gameplay/Variacao.h"

/**
 * CAMADA PURA. Coberto por test_elixir.cpp.
 *
 * NO ELIXIR TUDO E PROCESSO, e quem cuida dos processos e um SUPERVISOR. A
 * fase e a arvore de supervisao desenhada: o professor e o supervisor, e
 * embaixo dele, ligados por linhas, kWorkers processos - os workers, que o
 * aluno pode derrubar.
 *
 * O SUPERVISOR MANDA TRABALHO: a cada kEntreMensagens um GenServer.cast para
 * um worker, e a mensagem corre pela linha ate a caixa de correio dele. Com
 * kCaixa mensagens na caixa, o worker as processa (handle_cast) e atira no
 * aluno. O placar escreve cada cast.
 *
 * DERRUBAR UM WORKER E UM CRASH, e o supervisor o reinicia - conforme a
 * ESTRATEGIA desta rodada, que o placar diz embaixo:
 *
 *   :one_for_one    so o que caiu reinicia
 *   :one_for_all    TODOS terminam - cada um estoura num anel - e reiniciam
 *   :rest_for_one   o que caiu e os que vieram depois dele
 *
 * "Let it crash": derrubar abre o caminho por um instante, e o supervisor o
 * fecha. E no :one_for_all, derrubar um custa caro.
 */
namespace Elixir {

    constexpr const char* kFaseDoElixir = "StateThree";
    constexpr const char* kPostoDosWorkers = "supervisao";
    constexpr const char* kGatilhoDoWorker = "worker";      ///< o worker que o supervisor (re)inicia
    constexpr const char* kGatilhoDoCast = "cast";          ///< a mensagem correndo pela linha (enfeite)
    constexpr const char* kGatilhoDoHandle = "handle_cast"; ///< o leque do worker que processa a caixa
    constexpr const char* kGatilhoDoCrash = "crash";        ///< o anel do worker que termina

    // ---- A ARVORE ------------------------------------------------------------------

    constexpr int kWorkers = 4;
    constexpr float kAbaixo = 170.0f;          ///< os workers, abaixo do supervisor
    constexpr float kEntreWorkers = 190.0f;

    /// @brief O lugar do worker 'k', com o supervisor em ('x', 'y'), numa
    /// janela de 'largura': lado a lado embaixo dele, dentro do campo.
    std::array<float, 2> Lugar(int k, float x, float y, float largura);

    // ---- A ESTRATEGIA --------------------------------------------------------------

    enum class Estrategia { UmPorUm, TodosPorUm, RestoPorUm };

    /// ":one_for_one", ":one_for_all" ou ":rest_for_one".
    std::string Nome(Estrategia e);

    /// @brief Quem termina e reinicia quando o worker 'caido' cai: ele, todos,
    /// ou ele e os que vieram depois. Em ordem.
    std::vector<int> Reiniciam(Estrategia e, int caido, int workers = kWorkers);

    /// @brief As estrategias das rodadas desta luta: as tres numa ordem
    /// sorteada, e de novo.
    std::array<Estrategia, 3> Sortear(Variacao::Sorteio& s);

    // ---- O TRABALHO ----------------------------------------------------------------

    /// @brief Para quem vai o proximo cast: um worker vivo sorteado, que nao seja
    /// o do cast anterior (se houver outro). 'vivos' sao os indices dos workers
    /// em campo; sem nenhum, -1.
    int Destino(Variacao::Sorteio& s, const std::vector<int>& vivos, int anterior);

    /// O placar: o cast, o handle_cast, o crash e o restart, e embaixo a
    /// estrategia da rodada.
    std::string Cast(int worker, Estrategia e);
    std::string Handle(int worker, Estrategia e);
    std::string Crash(int worker, Estrategia e);

    // ---- O RELOGIO -----------------------------------------------------------------

    constexpr float kPrimeira = 1.0f;
    constexpr float kRodada = 20.0f / 3.0f;      ///< uma estrategia por rodada
    constexpr float kEntreMensagens = 0.45f;
    constexpr float kVooDaMensagem = 0.45f;      ///< do supervisor a caixa
    constexpr int kCaixa = 3;
    constexpr float kReinicio = 1.0f;            ///< do crash ao worker novo

    /// O PIPE: parado, o supervisor manda as duvidas pelo |>, e cada etapa
    /// escorre reta embaixo dele. A parada e a melhor hora de mirar nele - e
    /// ficar embaixo dele parado custa.
    constexpr const char* kGatilhoDoPipe = "pipe";
    constexpr int kEtapasDoPipe = 5;
    constexpr float kEntreEtapas = 0.1f;

    /// "duvidas\n|> Enum.map(&responde/1)": o placar da parada.
    std::string Pipe();

    /// @brief O que o Dificil muda: um cast a cada 0,35 s (e nao 0,45), e o
    /// pipe com 6 etapas.
    struct Degrau {
        float entreMensagens = kEntreMensagens;
        int etapas = kEtapasDoPipe;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
