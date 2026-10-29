//
// As transacoes da fase 3 do Thiago.
//

#pragma once

#include "../../Gameplay/Dificuldade.h"
#include "../../Gameplay/Variacao.h"

#include <cstdint>
#include <string>
#include <vector>

/**
 * CAMADA PURA.
 *
 * A FASE 3 DO THIAGO SAO TRANSACOES, varias ao mesmo tempo, sobre a tabela do
 * campo. Cada uma (T1, T2, T3...) faz dois UPDATE, em duas linhas, e termina:
 *
 *   UPDATE     a linha acende e e varrida - gravada -, e as escritas saem do
 *              professor e FICAM nela, em fila no meio, paradas: a linha esta
 *              TRAVADA (o lock) e o valor novo esta escrito em vermelho -
 *              ainda nao e de verdade
 *   COMMIT     as escritas somem, e o valor novo fica
 *   ROLLBACK   a linha acende e e varrida DO OUTRO LADO - o log lido de tras
 *              para a frente -, as escritas VOLTAM pelo mesmo caminho ate de
 *              onde sairam, e o valor antigo volta
 *
 * O LOCK E POR LINHA: uma transacao que quer uma linha travada por outra
 * ESPERA por ela ("T3 espera T2"). Se duas se esperam, e DEADLOCK: o banco
 * aborta a mais nova, que desfaz tudo (ROLLBACK), e a outra segue.
 *
 * Quem desenha, atira e desfaz e o Thiago; aqui ficam quem trava o que, quem
 * espera quem, o ciclo de espera, a vitima e os textos. Coberto por
 * test_transacao.cpp.
 */
namespace Transacao {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDaTransacao = "StateThree";
    constexpr const char* kGatilhoDaEscrita = "gravar";
    constexpr const char* kGatilhoDoDesfazer = "desfazer";

    /// Quantas escritas travam uma linha: em fila no meio dela, de ponta a ponta.
    constexpr int kEscritasPorLinha = 10;

    struct Tx {
        int id = 0;
        std::vector<int> segura;   ///< as linhas travadas por ela
        int quer = -1;             ///< a linha por que ela espera, ou -1
    };
    using Txs = std::vector<Tx>;

    /// @brief O id da transacao que trava a linha, ou 0 se ela esta livre.
    int Dono(const Txs& txs, int linha);

    /// @brief O ciclo de espera que passa pela transacao 'id': segue "quem eu
    /// espero" a partir dela, e se voltar a ela, devolve os ids do caminho.
    /// Vazio se nao ha ciclo.
    std::vector<int> CicloDeEspera(const Txs& txs, int id);

    /// @brief A vitima de um deadlock: a transacao mais nova do ciclo (o maior
    /// id). E a que tem menos trabalho a perder.
    int Vitima(const std::vector<int>& ciclo);

    /// @brief Tira a transacao da lista - e com ela os locks dela.
    void Liberar(Txs& txs, int id);

    /// @brief Uma linha de 0 a linhas - 1 que ninguem trava, sorteada, que nao
    /// seja 'evitar'. -1 se nao ha.
    int LinhaLivre(const Txs& txs, int linhas, std::uint32_t sorteio, int evitar = -1);

    /// @brief Uma linha travada por OUTRA transacao que nao 'id'. -1 se nao ha.
    int LinhaDeOutra(const Txs& txs, int id, std::uint32_t sorteio);

    /// @brief "T3: UPDATE SET nota=12 WHERE mat=42", "T3: COMMIT",
    /// "T3: ROLLBACK", "T3 espera T2", "deadlock! T3: ROLLBACK".
    std::string PlacarDoUpdate(int id, int nota, int mat);
    std::string PlacarDoFim(int id, bool desfaz);
    std::string PlacarDaEspera(int id, int dono);
    std::string PlacarDoDeadlock(int vitima);

    /// O relogio: a primeira transacao, uma nova a cada kNovaTransacao (ate
    /// kMaximo ao mesmo tempo), o intervalo entre os dois UPDATE, e quanto a
    /// transacao segura a segunda linha antes de terminar. O aviso de cada
    /// UPDATE e de cada ROLLBACK e o da linha da tabela (Banco::kAvisoDaLinha):
    /// sao varreduras de linha como as outras.
    constexpr float kPrimeira = 1.0f;
    constexpr float kNovaTransacao = 1.8f;
    constexpr int kMaximo = 3;
    constexpr float kEntreUpdates = 1.0f;
    constexpr float kSegurando = 1.4f;

    /// A chance de o segundo UPDATE pedir uma linha de outra transacao (e
    /// esperar por ela), e a de a transacao terminar em ROLLBACK.
    constexpr float kChanceDeDisputa = 0.5f;
    constexpr float kChanceDeRollback = 0.4f;

    /// A velocidade das escritas, na ida e na volta.
    constexpr float kVelocidadeDaEscrita = 450.0f;

    // ---- O QUE MUDA DE UMA LUTA PARA A OUTRA, E O DEGRAU -------------------

    /// As transacoes desta luta: com que chance o segundo UPDATE disputa uma
    /// linha (de 0,35 a 0,65 - com mais, mais espera e mais deadlock) e com
    /// que chance a transacao termina em ROLLBACK (de 0,25 a 0,55).
    struct Plano {
        float chanceDeDisputa = kChanceDeDisputa;
        float chanceDeRollback = kChanceDeRollback;
    };
    Plano Sortear(Variacao::Sorteio& s);

    /// @brief A transacao termina desfazendo? Com a chance do plano.
    bool Desfaz(std::uint32_t sorteio, float chance = kChanceDeRollback);

    /// O que o Dificil muda: uma transacao nova a cada 1,2 s, ate quatro de
    /// uma vez (e nao tres), as escritas indo e voltando a 600 px/s, e doze
    /// delas travando cada linha - passar entre elas fica mais apertado.
    struct Degrau {
        float novaTransacao = kNovaTransacao;
        int maximo = kMaximo;
        float velocidadeDaEscrita = kVelocidadeDaEscrita;
        int escritasPorLinha = kEscritasPorLinha;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
