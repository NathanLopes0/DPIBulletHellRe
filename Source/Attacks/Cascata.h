//
// O modelo cascata da fase 1 da Maria Lucia.
//

#pragma once

#include <array>
#include <cstdint>
#include <utility>
#include <vector>

#include "../Math.h"
#include "Fase.h"

/**
 * CAMADA PURA.
 *
 * A FASE 1 DA MARIA LUCIA E O MODELO CASCATA, e numa cascata O QUE UMA ETAPA
 * ENTREGA E O QUE A SEGUINTE RECEBE. Cinco etapas de 4 s, em ordem, e o que
 * sai de cada uma se ve passando para a outra:
 *
 *   1 Requisitos     o cliente pede: post-its chegam pelos lados do campo e
 *                    colam no quadro do backlog, ao lado da professora
 *   2 Projeto        o backlog vira o DIAGRAMA: cada post-it voa para o meio do
 *                    campo e vira uma CLASSE, as classes se ligam por setas, e
 *                    as mensagens saem delas pelas setas, virando em angulo reto
 *   3 Implementacao  cada classe compila: solta codigo mirado no aluno
 *   4 Testes         cada classe solta o bug dela
 *   5 Manutencao     o sistema e entregue - as classes somem -, e cada bug que
 *                    o teste nao achou vira DIVIDA, parada no campo
 *
 * O TIRO DO ALUNO E UMA DECISAO DE PROCESSO: o post-it derrubado no caminho e
 * escopo cortado - uma classe a menos, menos codigo, um bug a menos -, e o bug
 * derrubado nos Testes nao vira divida.
 *
 * Quem desenha, move e atira e a Maria Lucia; aqui ficam as etapas, os lugares
 * do backlog e das classes, as setas do diagrama e os relogios. Coberto por
 * test_cascata.cpp.
 */
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

    /// @brief Em que etapa se esta no instante 't' da fase: 0 a 4, e 5 depois
    /// da ultima.
    int EtapaNoTempo(float t);

    // ---- REQUISITOS --------------------------------------------------------

    /// Um pedido a cada kEntrePedidos, ate o backlog encher (kMaxPedidos).
    constexpr float kEntrePedidos = 0.3f;
    constexpr float kVelocidadeDoPedido = 300.0f;
    constexpr int kMaxPedidos = 10;

    /// @brief O lugar do pedido k no quadro do backlog: duas filas de cinco,
    /// debaixo da caixa da primeira etapa.
    Vector2 LugarNoBacklog(int k);

    /// @brief De onde vem um pedido: de um dos lados do campo, numa altura
    /// sorteada da metade de baixo - o cliente esta do lado de fora.
    Vector2 OrigemDoPedido(std::uint32_t sorteio, float largura, float altura);

    // ---- PROJETO -----------------------------------------------------------

    /// As classes cabem numa grade de 4 x 2 no meio do campo, um pouco para a
    /// esquerda: a 0,2 da largura, a ultima da primeira fila encostava na caixa
    /// da Manutencao.
    constexpr int kMaxClasses = 8;
    constexpr int kClassesPorLinha = 4;
    constexpr float kYDasClasses = 430.0f;
    constexpr float kEntreLinhasDeClasses = 95.0f;

    /// @brief O lugar da classe k, numa tela de 'largura'.
    Vector2 LugarDaClasse(int k, float largura);

    /// @brief As setas do diagrama de 'classes' classes: cada uma liga a
    /// vizinha da mesma fila e a de baixo dela. Pares (de, para).
    std::vector<std::pair<int, int>> Associacoes(int classes);

    /// As mensagens: uma classe por vez solta as suas, a cada kEntreMensagens,
    /// depois de o diagrama ficar pronto (kDiagramaPronto depois de a etapa
    /// abrir).
    constexpr float kDiagramaPronto = 0.7f;
    constexpr float kEntreMensagens = 0.75f;

    // ---- IMPLEMENTACAO E TESTES ---------------------------------------------

    /// Uma classe por vez compila, a cada kEntreCompilacoes.
    constexpr float kEntreCompilacoes = 0.28f;

    // ---- MANUTENCAO --------------------------------------------------------

    /// Quanto a divida fica parada no campo - atravessa a troca de fase.
    constexpr float kDivida = 8.5f;

    /// A caixa de cada etapa fica kAbaixoDoDegrau abaixo do degrau dela: longe
    /// o bastante para a professora, de pe no degrau, nao a cobrir.
    constexpr float kAbaixoDoDegrau = 70.0f;
}
