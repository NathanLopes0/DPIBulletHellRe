//
// O problema da parada: a repescagem do Vladimir.
//

#include "doctest.h"

#include <algorithm>
#include <set>

#include "../Source/Attacks/Parada.h"

TEST_CASE("Parada: um passo do programa de Collatz, e o 1 fica") {
    CHECK(Parada::Proximo(6) == 3);
    CHECK(Parada::Proximo(3) == 10);
    CHECK(Parada::Proximo(1) == 1);
    CHECK(Parada::Proximo(0) == 0);
    CHECK(Parada::Parou(1));
    CHECK_FALSE(Parada::Parou(2));
    // 7 -> 22 -> 11 -> 34 -> 17 -> 52 -> 26 -> 13 -> 40 -> 20 -> 10 -> 5 -> 16 -> 8 -> 4 -> 2 -> 1
    CHECK(Parada::PassosAteParar(7) == 16);
    CHECK(Parada::PassosAteParar(27, 50) == 50);   // o limite segura
}

TEST_CASE("Parada: toda semente para em 14 a 29 passos, sem passar de 400 no caminho") {
    for (const auto s : Parada::kSementes) {
        CAPTURE(s);
        const int passos = Parada::PassosAteParar(s);
        CHECK(passos >= 14);
        CHECK(passos <= 29);
        std::uint32_t x = s, maior = s;
        while (!Parada::Parou(x)) { x = Parada::Proximo(x); maior = std::max(maior, x); }
        CHECK(maior <= 400u);
    }
    // O sorteio so devolve sementes da lista, e chega a todas.
    std::set<std::uint32_t> vistas;
    for (std::uint32_t i = 0; i < 200; ++i) vistas.insert(Parada::Sortear(i * 2654435761u));
    CHECK(vistas.size() == Parada::kSementes.size());
}

TEST_CASE("Parada: a altura sobe com x, do 1 embaixo ao teto em cima") {
    CHECK(Parada::Altura(1) == doctest::Approx(Parada::kBaixo));
    CHECK(Parada::Altura(256) == doctest::Approx(Parada::kAlto));
    CHECK(Parada::Altura(10000) == doctest::Approx(Parada::kAlto));   // nao passa do teto
    CHECK(Parada::Altura(16) == doctest::Approx((Parada::kAlto + Parada::kBaixo) / 2.0f));
    CHECK(Parada::Altura(40) < Parada::Altura(5));                    // em cima e y menor
}

TEST_CASE("Parada: o placar diz o passo que vem") {
    CHECK(Parada::Placar(41) == "x = 41: 3x + 1");
    CHECK(Parada::Placar(124) == "x = 124: x / 2");
    CHECK(Parada::Placar(1) == "x = 1: parou");
}
