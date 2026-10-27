//
// A repescagem da Maria Lucia: nao ha bala de prata.
//

#include "doctest.h"

#include <cstdint>
#include <set>

#include "../Source/Attacks/BalaDePrata.h"

TEST_CASE("BalaDePrata: cada luta embaralha as promessas, todas, sem repetir") {
    const size_t n = BalaDePrata::Promessas().size();
    REQUIRE(n >= 6u);
    std::set<int> primeiras;
    for (std::uint32_t semente = 0; semente < 200; ++semente) {
        Variacao::Sorteio s = Variacao::SorteioDa(semente, "StateFinal");
        const auto ordem = BalaDePrata::Sortear(s);
        REQUIRE(ordem.size() == n);
        CHECK(std::set<int>(ordem.begin(), ordem.end()).size() == n);
        primeiras.insert(ordem.front());
    }
    // A PRIMEIRA BALA NAO E SEMPRE A MESMA.
    CHECK(primeiras.size() == n);
}

TEST_CASE("BalaDePrata: o placar diz que nao ha bala de prata, ou que o teste a reprovou") {
    CHECK(BalaDePrata::Desfeita("IA") == "IA? N\xc3\xa3o h\xc3\xa1 bala de prata.");
    CHECK(BalaDePrata::Testada("UML") == "UML \xc2\xb7 teste com usu\xc3\xa1rio: reprovada");
}

TEST_CASE("BalaDePrata: o degrau do Normal e o de sempre, e o Dificil dispara mais") {
    CHECK(BalaDePrata::DegrauDo(Dificuldade::Nivel::Normal).entreBalas == doctest::Approx(1.7f));
    CHECK(BalaDePrata::DegrauDo(Dificuldade::Nivel::Dificil).entreBalas <
          BalaDePrata::DegrauDo(Dificuldade::Nivel::Normal).entreBalas);
}

TEST_CASE("BalaDePrata: a bala se desfaz perto do aluno, ou no fim do voo se ele fugiu") {
    CHECK_FALSE(BalaDePrata::SeDesfaz(BalaDePrata::kPerto + 1.0f, 0.5f));
    CHECK(BalaDePrata::SeDesfaz(BalaDePrata::kPerto, 0.5f));
    CHECK(BalaDePrata::SeDesfaz(BalaDePrata::kPerto + 300.0f, BalaDePrata::kVooMaximo));
}
