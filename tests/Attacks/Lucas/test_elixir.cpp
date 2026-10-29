//
// A fase 3 do Lucas: a arvore de supervisao do Elixir.
//

#include "../../doctest.h"

#include <cstdint>
#include <set>

#include "../../../Source/Attacks/Lucas/Elixir.h"

using namespace Elixir;

TEST_CASE("Elixir: cada estrategia reinicia quem o OTP reinicia") {
    CHECK(Reiniciam(Estrategia::UmPorUm, 2) == std::vector<int>{2});
    CHECK(Reiniciam(Estrategia::TodosPorUm, 2) == std::vector<int>{0, 1, 2, 3});
    CHECK(Reiniciam(Estrategia::RestoPorUm, 1) == std::vector<int>{1, 2, 3});
    CHECK(Reiniciam(Estrategia::RestoPorUm, 3) == std::vector<int>{3});
    CHECK(Nome(Estrategia::TodosPorUm) == ":one_for_all");
}

TEST_CASE("Elixir: os workers ficam lado a lado embaixo do supervisor, dentro da janela") {
    for (const float x : {0.0f, 300.0f, 600.0f, 1200.0f}) {
        float anterior = -1.0f;
        for (int k = 0; k < kWorkers; ++k) {
            const auto p = Lugar(k, x, 150.0f, 1200.0f);
            CHECK(p[0] > 0.0f);
            CHECK(p[0] < 1200.0f);
            CHECK(p[1] == doctest::Approx(150.0f + kAbaixo));
            if (k > 0) CHECK(p[0] - anterior == doctest::Approx(kEntreWorkers));
            anterior = p[0];
        }
    }
    CHECK(Lugar(0, 600.0f, 150.0f, 1200.0f)[0] < 600.0f);
    CHECK(Lugar(kWorkers - 1, 600.0f, 150.0f, 1200.0f)[0] > 600.0f);
}

TEST_CASE("Elixir: cada luta tem as tres estrategias, numa ordem propria, e o cast nao repete o worker") {
    std::set<int> primeiras;
    for (std::uint32_t semente = 0; semente < 60; ++semente) {
        Variacao::Sorteio s = Variacao::SorteioDa(semente, kFaseDoElixir);
        const auto plano = Sortear(s);
        std::set<int> todas;
        for (const auto e : plano) todas.insert(static_cast<int>(e));
        CHECK(todas.size() == 3u);
        primeiras.insert(static_cast<int>(plano[0]));
    }
    CHECK(primeiras.size() == 3u);

    Variacao::Sorteio s = Variacao::SorteioDa(5u, kFaseDoElixir);
    int anterior = -1;
    for (int k = 0; k < 100; ++k) {
        const int d = Destino(s, {0, 1, 2, 3}, anterior);
        CHECK(d != anterior);
        anterior = d;
    }
    CHECK(Destino(s, {2}, 2) == 2);   // so um vivo: e ele
    CHECK(Destino(s, {}, 0) == -1);
}

TEST_CASE("Elixir: o placar e o codigo do Elixir, com a estrategia embaixo") {
    CHECK(Cast(1, Estrategia::UmPorUm) == "GenServer.cast(:w2, :duvida)\nstrategy: :one_for_one");
    CHECK(Crash(0, Estrategia::TodosPorUm) == "** (EXIT) :w1 killed\nstrategy: :one_for_all");
    CHECK(Handle(3, Estrategia::RestoPorUm).find("handle_cast") != std::string::npos);
    CHECK(DegrauDo(Dificuldade::Nivel::Dificil).entreMensagens < kEntreMensagens);
}
