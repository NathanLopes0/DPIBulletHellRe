//
// O modelo cascata da fase 1 da Maria Lucia.
//

#pragma once

#include <array>
#include <cstdint>
#include <utility>
#include <vector>

#include "../Motor/Math.h"
#include "../Regras/Variacao.h"
#include "Fase.h"

/// A fase 1 da Maria Lucia e o modelo cascata, e numa cascata o que uma etapa entrega e o que a
/// seguinte recebe.
namespace Cascata {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDaCascata = "StateOne";
    constexpr std::array<const char*, 5> kEtapas = {"Requisitos", "Projeto", "Implementacao", "Testes", "Manutencao"};
    constexpr const char* kGatilhoDaPlaca = "placa";
    constexpr const char* kGatilhoDoPedido = "pedido";
    constexpr const char* kGatilhoDaClasse = "classe";
    constexpr const char* kGatilhoDaMensagem = "mensagem";
    constexpr const char* kGatilhoDoCodigo = "compilar";
    constexpr const char* kGatilhoDoBug = "bug";
    constexpr const char* kGatilhoDaDivida = "debito";

    /// A duracao de cada etapa: a mesma das "etapas" de fases.json e dos
    /// degraus da escada (um teste confere).
    constexpr float kDuracaoDaEtapa = Fase::kDuracao / static_cast<float>(kEtapas.size());

    /// Em que etapa se esta no instante 't' da fase: 0 a 4, e 5 depois da ultima.
    int EtapaNoTempo(float t);

    // ---- Requisitos ----

    /// Um pedido a cada kEntrePedidos, ate o backlog encher (kMaxPedidos).
    constexpr float kEntrePedidos = 0.3f;
    constexpr float kVelocidadeDoPedido = 300.0f;
    constexpr int kMaxPedidos = 10;

    /// O lugar do pedido k no quadro do backlog: duas filas de cinco, debaixo da caixa da primeira
    /// etapa.
    Vector2 LugarNoBacklog(int k);

    /// De onde vem um pedido: de um dos lados do campo, numa altura sorteada da metade de baixo - o
    /// cliente esta do lado de fora.
    Vector2 OrigemDoPedido(std::uint32_t sorteio, float largura, float altura);

    // ---- Projeto ----

    /// As classes cabem numa grade de 4 x 2 no meio do campo, um pouco para a esquerda.
    constexpr int kMaxClasses = 8;
    constexpr int kClassesPorLinha = 4;
    constexpr float kYDasClasses = 430.0f;
    constexpr float kEntreLinhasDeClasses = 95.0f;

    /// O lugar da classe k, numa tela de 'largura'.
    Vector2 LugarDaClasse(int k, float largura);

    /// As setas do diagrama de 'classes' classes: cada uma liga a vizinha da mesma fila e a de
    /// baixo dela. Pares (de, para).
    std::vector<std::pair<int, int>> Associacoes(int classes);

    /// As mensagens: uma classe por vez solta as suas, a cada kEntreMensagens,
    /// depois de o diagrama ficar pronto (kDiagramaPronto depois de a etapa
    /// abrir).
    constexpr float kDiagramaPronto = 0.7f;
    constexpr float kEntreMensagens = 0.75f;

    // ---- Implementacao e testes ----

    /// Uma classe por vez compila, a cada kEntreCompilacoes.
    constexpr float kEntreCompilacoes = 0.28f;

    // ---- O que muda de uma luta para a outra ----

    /// A cascata desta luta: em que ordem as classes do diagrama soltam as mensagens e compilam.
    struct Plano {
        std::vector<int> ordem;
    };
    Plano Sortear(Variacao::Sorteio& s);

    /// Qual das 'prontas' classes tem a vez 'vez': a vez-esima da ordem do plano entre as que
    /// existem, e a volta seguinte repete a ordem.
    int DaVez(const Plano& p, int vez, int prontas);

    // ---- Manutencao ----

    /// Quanto a divida fica parada no campo - atravessa a troca de fase.
    constexpr float kDivida = 8.5f;

    /// A caixa de cada etapa fica kAbaixoDoDegrau abaixo do degrau dela: longe
    /// o bastante para a professora, de pe no degrau, nao a cobrir.
    constexpr float kAbaixoDoDegrau = 70.0f;
}
