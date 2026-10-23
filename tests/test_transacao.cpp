//
// As transacoes da fase 3 do Thiago.
//

#include "doctest.h"

#include <set>

#include "../Source/Attacks/Transacao.h"

TEST_CASE("Transacao: o dono de uma linha e quem a trava") {
    const Transacao::Txs txs = {{1, {0, 2}, -1}, {2, {3}, -1}};
    CHECK(Transacao::Dono(txs, 0) == 1);
    CHECK(Transacao::Dono(txs, 2) == 1);
    CHECK(Transacao::Dono(txs, 3) == 2);
    CHECK(Transacao::Dono(txs, 1) == 0);
}

TEST_CASE("Transacao: duas que se esperam sao um deadlock, e a vitima e a mais nova") {
    // T1 trava a 0 e quer a 1; T2 trava a 1 e quer a 0.
    const Transacao::Txs txs = {{1, {0}, 1}, {2, {1}, 0}};
    const auto ciclo = Transacao::CicloDeEspera(txs, 2);
    CHECK(std::set<int>(ciclo.begin(), ciclo.end()) == std::set<int>{1, 2});
    CHECK(Transacao::Vitima(ciclo) == 2);
    CHECK(!Transacao::CicloDeEspera(txs, 1).empty());
}

TEST_CASE("Transacao: esperar quem nao espera ninguem nao e deadlock") {
    const Transacao::Txs txs = {{1, {0}, -1}, {2, {1}, 0}, {3, {2}, 1}};
    CHECK(Transacao::CicloDeEspera(txs, 3).empty());   // 3 espera 2, que espera 1, que nao espera
    CHECK(Transacao::CicloDeEspera(txs, 2).empty());
    CHECK(Transacao::CicloDeEspera(txs, 1).empty());
    // Uma linha livre, ou a propria, tambem nao.
    const Transacao::Txs sozinha = {{4, {1}, 3}};
    CHECK(Transacao::CicloDeEspera(sozinha, 4).empty());
}

TEST_CASE("Transacao: um ciclo de tres tambem e deadlock") {
    const Transacao::Txs txs = {{1, {0}, 1}, {2, {1}, 2}, {3, {2}, 0}};
    const auto ciclo = Transacao::CicloDeEspera(txs, 1);
    CHECK(ciclo.size() == 3);
    CHECK(Transacao::Vitima(ciclo) == 3);
    // Quem espera um ciclo, sem fazer parte dele, nao esta nele.
    const Transacao::Txs fora = {{1, {0}, 1}, {2, {1}, 0}, {3, {2}, 0}};
    CHECK(Transacao::CicloDeEspera(fora, 3).empty());
}

TEST_CASE("Transacao: liberar tira a transacao e os locks dela") {
    Transacao::Txs txs = {{1, {0}, -1}, {2, {1}, 0}};
    Transacao::Liberar(txs, 1);
    REQUIRE(txs.size() == 1);
    CHECK(Transacao::Dono(txs, 0) == 0);   // quem esperava a 0 pode seguir
    CHECK(Transacao::Dono(txs, 1) == 2);
}

TEST_CASE("Transacao: a linha livre e livre, e a de outra e de outra") {
    const Transacao::Txs txs = {{1, {0, 2}, -1}, {2, {3}, -1}};
    for (std::uint32_t s = 0; s < 200; ++s) {
        CHECK(Transacao::LinhaLivre(txs, 4, s) == 1);
        CHECK(Transacao::LinhaLivre(txs, 4, s, 1) == -1);
        const int deOutra = Transacao::LinhaDeOutra(txs, 2, s);
        CHECK((deOutra == 0 || deOutra == 2));
        CHECK(Transacao::LinhaDeOutra(txs, 1, s) == 3);
    }
    CHECK(Transacao::LinhaDeOutra({{1, {0}, -1}}, 1, 7u) == -1);
}

TEST_CASE("Transacao: o ROLLBACK sai com a chance dele") {
    int desfeitas = 0;
    for (std::uint32_t s = 0; s < 2000; ++s) desfeitas += Transacao::Desfaz(s * 2654435761u) ? 1 : 0;
    CHECK(desfeitas > 2000 * (Transacao::kChanceDeRollback - 0.06f));
    CHECK(desfeitas < 2000 * (Transacao::kChanceDeRollback + 0.06f));
}

TEST_CASE("Transacao: os placares") {
    CHECK(Transacao::PlacarDoUpdate(3, 12, 42) == "T3: UPDATE SET nota=12 WHERE mat=42");
    CHECK(Transacao::PlacarDoFim(3, false) == "T3: COMMIT");
    CHECK(Transacao::PlacarDoFim(4, true) == "T4: ROLLBACK");
    CHECK(Transacao::PlacarDaEspera(3, 2) == "T3 espera T2");
    CHECK(Transacao::PlacarDoDeadlock(5) == "deadlock! T5: ROLLBACK");
}
