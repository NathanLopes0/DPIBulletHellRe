//
// As transacoes da fase 3 do Thiago.
//

#pragma once

#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

#include <cstdint>
#include <string>
#include <vector>

/// A fase 3 do Thiago sao transacoes, varias ao mesmo tempo, sobre a tabela do campo.
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

    /// O id da transacao que trava a linha, ou 0 se ela esta livre.
    int Dono(const Txs& txs, int linha);

    /// O ciclo de espera que passa pela transacao 'id': segue "quem eu espero" a partir dela, e se
    /// voltar a ela, devolve os ids do caminho.
    std::vector<int> CicloDeEspera(const Txs& txs, int id);

    /// A vitima de um deadlock: a transacao mais nova do ciclo (o maior id). E a que tem menos
    /// trabalho a perder.
    int Vitima(const std::vector<int>& ciclo);

    /// Tira a transacao da lista - e com ela os locks dela.
    void Liberar(Txs& txs, int id);

    /// Uma linha de 0 a linhas - 1 que ninguem trava, sorteada, que nao seja 'evitar'. -1 se nao
    /// ha.
    int LinhaLivre(const Txs& txs, int linhas, std::uint32_t sorteio, int evitar = -1);

    /// Uma linha travada por outra transacao que nao 'id'. -1 se nao ha.
    int LinhaDeOutra(const Txs& txs, int id, std::uint32_t sorteio);

    /// "T3: UPDATE SET nota=12 WHERE mat=42", "T3: COMMIT", "T3: ROLLBACK", "T3 espera T2",
    /// "deadlock! T3: rollback".
    std::string PlacarDoUpdate(int id, int nota, int mat);
    std::string PlacarDoFim(int id, bool desfaz);
    std::string PlacarDaEspera(int id, int dono);
    std::string PlacarDoDeadlock(int vitima);

    /// O aviso de cada update e de cada rollback e o da linha da tabela (Banco::kAvisoDaLinha): sao
    /// varreduras de linha como as outras.
    constexpr float kPrimeira = 1.0f;
    constexpr float kNovaTransacao = 1.8f;
    constexpr int kMaximo = 3;
    constexpr float kEntreUpdates = 1.0f;
    constexpr float kSegurando = 1.4f;

    /// A chance de o segundo update pedir uma linha de outra transacao (e esperar por ela), e a de
    /// a transacao terminar em rollback.
    constexpr float kChanceDeDisputa = 0.5f;
    constexpr float kChanceDeRollback = 0.4f;

    /// A velocidade das escritas, na ida e na volta.
    constexpr float kVelocidadeDaEscrita = 450.0f;

    // ---- O que muda de uma luta para a outra, e o degrau ----

    /// As transacoes desta luta: com que chance o segundo update disputa uma linha (de 0,35 a 0,65.
    struct Plano {
        float chanceDeDisputa = kChanceDeDisputa;
        float chanceDeRollback = kChanceDeRollback;
    };
    Plano Sortear(Variacao::Sorteio& s);

    /// A transacao termina desfazendo? Com a chance do plano.
    bool Desfaz(std::uint32_t sorteio, float chance = kChanceDeRollback);

    struct Degrau {
        float novaTransacao = kNovaTransacao;
        int maximo = kMaximo;
        float velocidadeDaEscrita = kVelocidadeDaEscrita;
        int escritasPorLinha = kEscritasPorLinha;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
