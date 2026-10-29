//
// A repescagem do Lucas: o event loop do JavaScript.
//

#pragma once

#include <array>
#include <string>
#include <vector>

#include "../../Gameplay/Dificuldade.h"
#include "../../Gameplay/Variacao.h"

/**
 * CAMADA PURA. Coberto por test_event_loop.cpp.
 *
 * O JAVASCRIPT TEM UMA THREAD SO, e quem decide o que roda e o EVENT LOOP. A
 * repescagem e ele desenhado: a pilha de chamadas no meio, as Web APIs (os
 * timers e o fetch) no alto a esquerda, a fila de microtarefas embaixo dela e a
 * de tarefas do outro lado. O professor e o loop, girando no alto.
 *
 * Cada rodada e um SCRIPT gerado - console.log, setTimeout, Promise.then,
 * fetch -, e ele roda na ordem que o JavaScript de verdade roda (Simular):
 *
 *   o script passa pela pilha, uma linha por vez;
 *   o setTimeout vai para as Web APIs, e o callback so entra na fila de
 *   tarefas quando o timer acaba - mesmo com 0 ms;
 *   o .then entra na fila de MICROtarefas;
 *   pilha vazia: o loop roda TODAS as microtarefas, e so entao UMA tarefa - e
 *   as microtarefas de novo.
 *
 * Por isso o setTimeout(cb, 0) escrito ANTES do .then roda DEPOIS dele - e o
 * aluno que sabe disso sabe o que vem. Cada coisa que roda ataca do seu jeito:
 * o console.log atira, o .then solta promessas, a resposta do fetch e uma
 * promessa que resolve ou e rejeitada, e o callback do setTimeout renderiza
 * uma caixa.
 */
namespace EventLoop {

    constexpr const char* kFaseDoLoop = "StateFinal";
    constexpr const char* kPostoDoLoop = "eventloop";
    constexpr const char* kGatilhoDaFicha = "ficha";      ///< o item numa fila ou na pilha (enfeite)
    constexpr const char* kGatilhoDoLog = "log";
    constexpr const char* kGatilhoDoThen = "then";
    constexpr const char* kGatilhoDoFetch = "fetch";
    constexpr const char* kGatilhoDoTimeout = "timeout";

    enum class Tipo { Log, Timeout, Then, Fetch };

    struct Linha {
        Tipo tipo = Tipo::Log;
        float atraso = 0.0f;     ///< segundos: o do setTimeout, ou o da resposta do fetch
        std::string nome;        ///< o callback: "cb1", "p1", "r1"; o texto do log
    };

    /// @brief Um script novo: de 5 a 6 linhas, com pelo menos um console.log,
    /// e um setTimeout(cb, 0) ESCRITO ANTES de um .then - a pegadinha.
    std::vector<Linha> Gerar(Variacao::Sorteio& s);

    /// @brief A linha como o codigo a escreve: "setTimeout(cb1, 0)".
    std::string Codigo(const Linha& l);

    /// @brief O placar quando a linha entra na pilha: o codigo e, embaixo,
    /// para onde vai o que ela deixa - "setTimeout(cb1, 0)" e "timer nas Web
    /// APIs".
    std::string Placar(const Linha& l);

    /// @brief O placar quando o loop busca o callback da linha: "cb1()" e
    /// "da fila de tarefas".
    std::string PlacarDoCallback(const Linha& l);

    enum class Fila { Nenhuma, Micro, Tarefa };

    /// O que o loop fez com cada linha que deixa alguma coisa para depois: a
    /// hora em que ela registrou, a hora em que o callback entrou na fila e a
    /// hora em que ele rodou. O log nao deixa nada.
    struct Item {
        int linha = 0;
        Fila fila = Fila::Nenhuma;
        float registrado = 0.0f;
        float naFila = 0.0f;
        float rodou = 0.0f;
    };

    /// Quando cada coisa roda na pilha: uma linha do script, ou o callback de
    /// um item.
    struct Execucao {
        float inicio = 0.0f;
        int linha = 0;
        bool callback = false;   ///< false: a linha do script; true: o callback dela
        Fila fila = Fila::Nenhuma;
    };

    struct Agenda {
        std::vector<Item> itens;
        std::vector<Execucao> execucoes;   ///< em ordem de inicio
        float fim = 0.0f;                  ///< quando a pilha e as filas ficam vazias
    };

    /// @brief O loop: cada linha ocupa a pilha 'porLinha' segundos, e cada
    /// callback 'porCallback'.
    Agenda Simular(const std::vector<Linha>& script, float porLinha, float porCallback);

    // ---- O DESENHO -------------------------------------------------------------------
    //
    // O loop e o professor: ele orbita (o movimento "Orbitar" da fase, raio
    // kRaioDoLoop) em volta da pilha. As Web APIs ficam no alto a esquerda; as
    // duas filas embaixo, fora da orbita, com a CABECA virada para a pilha.

    constexpr float kYDoLoop = 240.0f;        ///< o centro da orbita: 30% de 800
    constexpr float kRaioDoLoop = 200.0f;

    struct Retangulo {
        float x = 0.0f;
        float y = 0.0f;
        float w = 0.0f;
        float h = 0.0f;
    };
    struct Ponto {
        float x = 0.0f;
        float y = 0.0f;
    };

    enum class Lugar { Nenhum, Pilha, Apis, Micro, Tarefa };

    /// @brief As quatro caixas numa janela de 'largura', na ordem de Lugar sem
    /// o Nenhum: a pilha, as Web APIs, a fila de microtarefas e a de tarefas.
    std::array<Retangulo, 4> Caixas(float largura);

    /// @brief Onde fica a ficha que esta em 'lugar', na 'posicao' (a vaga nas
    /// Web APIs; nas filas, a partir da cabeca).
    Ponto Posicao(Lugar lugar, int posicao, float largura);

    /// Onde esta, no instante 't' do script, a ficha de uma linha.
    struct Estado {
        Lugar lugar = Lugar::Nenhum;   ///< Nenhum: ainda nao nasceu, ou ja saiu da pilha
        int posicao = 0;
        float falta = 0.0f;            ///< nas Web APIs: a fracao do timer que falta, de 1 a 0
    };

    /// @brief A ficha da linha 'linha' no instante 't' (segundos desde o
    /// comeco do script): na pilha enquanto a linha roda, nas Web APIs
    /// enquanto o timer corre, na fila ate o loop a buscar, e na pilha de novo
    /// enquanto o callback roda. Na fila, a posicao e a ordem em que o loop
    /// vai busca-la.
    Estado Onde(const std::vector<Linha>& script, const Agenda& a, int linha, float t,
                float porLinha, float porCallback);

    // ---- O RELOGIO -------------------------------------------------------------------

    constexpr float kPrimeiro = 0.8f;
    constexpr float kPorLinha = 0.45f;
    constexpr float kPorCallback = 0.55f;
    constexpr float kEntreScripts = 1.0f;

    /// @brief O que o Dificil muda: a pilha anda mais depressa (0,35 s por
    /// linha, 0,42 por callback).
    struct Degrau {
        float porLinha = kPorLinha;
        float porCallback = kPorCallback;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
