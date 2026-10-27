//
// A fase 3 do Lucas: processos do Elixir trocando mensagens.
//

#pragma once

#include <string>

#include "../Dificuldade.h"

/**
 * CAMADA PURA. Coberto por test_elixir.cpp.
 *
 * NO ELIXIR TUDO E PROCESSO, E PROCESSO SO CONVERSA POR MENSAGEM: send(pid,
 * msg). Os processos da fase 3 perseguem o aluno, como antes - e agora, a
 * cada kEntreMensagens, um deles manda uma mensagem para ele (o gatilho
 * "send"), um de cada vez, em rodizio, e o placar escreve o send. Antes o
 * processo so ia atras, devagar, e a fase era a mais parada da batalha.
 *
 * O SUPERVISOR CONTINUA: o processo derrubado volta na rodada seguinte
 * (Lucas::OnProjetilDerrubado). Derrubar um processo tambem cala as mensagens
 * dele.
 */
namespace Elixir {

    constexpr const char* kFaseDoElixir = "StateThree";
    constexpr const char* kGatilhoDaMensagem = "send";

    constexpr float kPrimeira = 1.0f;
    constexpr float kEntreMensagens = 0.6f;

    /// @brief Quem manda a 'vez'-esima mensagem, entre 'processos' vivos: um
    /// de cada vez, em rodizio. Sem processo, ninguem (-1).
    int Remetente(int vez, int processos);

    /// @brief O placar da 'n'-esima mensagem: "send(aluno, {:duvida, n})".
    std::string Placar(int n);

    /// @brief O que o Dificil muda: uma mensagem a cada 0,35 s (e nao 0,6).
    struct Degrau {
        float entreMensagens = kEntreMensagens;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
