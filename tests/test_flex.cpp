//
// O flexbox do CSS: as posicoes de cada justify-content e a curva da transicao.
//

#include "doctest.h"

#include "../Source/Attacks/Flex.h"

#include <vector>

// A PARTE QUE DA PARA TESTAR DE UMA ESTRATEGIA QUE CRIA PROJETEIS.
//
// FlexAttack pede projeteis ao pool e nao entra na suite. O que entra e a conta
// de onde cada item fica - que tem de bater com o CSS de verdade, porque o
// placar escreve o nome do valor e quem estudou a materia vai conferir.

TEST_CASE("Flex: os cinco valores de justify-content, numa linha de 0 a 600 com 4 itens de 50") {
    using Flex::Justificar;

    auto x = Flex::Posicoes(Justificar::Inicio, 4, 50.0f, 0.0f, 600.0f);
    CHECK(x == std::vector<float>{25.0f, 75.0f, 125.0f, 175.0f});

    x = Flex::Posicoes(Justificar::Fim, 4, 50.0f, 0.0f, 600.0f);
    CHECK(x == std::vector<float>{425.0f, 475.0f, 525.0f, 575.0f});

    x = Flex::Posicoes(Justificar::Centro, 4, 50.0f, 0.0f, 600.0f);
    CHECK(x == std::vector<float>{225.0f, 275.0f, 325.0f, 375.0f});

    // space-between: sobram 400, divididos em 3 vaos de 133,33.
    x = Flex::Posicoes(Justificar::EspacoEntre, 4, 50.0f, 0.0f, 600.0f);
    REQUIRE(x.size() == 4);
    CHECK(x[0] == doctest::Approx(25.0f));
    CHECK(x[1] == doctest::Approx(208.333f));
    CHECK(x[3] == doctest::Approx(575.0f));

    // space-around: 4 vaos de 100, meio vao (50) nas pontas.
    x = Flex::Posicoes(Justificar::EspacoEmVolta, 4, 50.0f, 0.0f, 600.0f);
    CHECK(x == std::vector<float>{75.0f, 225.0f, 375.0f, 525.0f});
}

TEST_CASE("Flex: um item so, ou nenhum, como o CSS faz") {
    using Flex::Justificar;
    // space-between com um item se comporta como flex-start.
    CHECK(Flex::Posicoes(Justificar::EspacoEntre, 1, 50.0f, 0.0f, 600.0f) == std::vector<float>{25.0f});
    CHECK(Flex::Posicoes(Justificar::Centro, 0, 50.0f, 0.0f, 600.0f).empty());
}

TEST_CASE("Flex: os nomes do CSS vao e voltam, e um nome inventado e recusado") {
    for (const char* nome : {"flex-start", "flex-end", "center", "space-between", "space-around"}) {
        CAPTURE(nome);
        Flex::Justificar j{};
        REQUIRE(Flex::PeloNome(nome, j));
        CHECK(Flex::Nome(j) == nome);
    }
    Flex::Justificar j{};
    CHECK_FALSE(Flex::PeloNome("space-evenly", j));
    CHECK_FALSE(Flex::PeloNome("centro", j));
}

TEST_CASE("Flex: a transicao comeca e termina devagar, e fica parada fora da janela") {
    CHECK(Flex::Progresso(0.3f, 0.6f, 0.5f) == doctest::Approx(0.0f));
    CHECK(Flex::Progresso(0.85f, 0.6f, 0.5f) == doctest::Approx(0.5f));
    CHECK(Flex::Progresso(1.2f, 0.6f, 0.5f) == doctest::Approx(1.0f));

    // EASE-IN-OUT: no primeiro decimo da janela andou menos que um decimo do
    // caminho, e no ultimo decimo andou menos que um decimo tambem.
    CHECK(Flex::Progresso(0.65f, 0.6f, 0.5f) < 0.1f);
    CHECK(Flex::Progresso(1.05f, 0.6f, 0.5f) > 0.9f);

    // Duracao zero: um degrau.
    CHECK(Flex::Progresso(0.6001f, 0.6f, 0.0f) == doctest::Approx(1.0f));
}
