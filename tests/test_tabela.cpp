// A tabela do Thiago.

#include "doctest.h"

#include "../Source/Regras/Tabela.h"

TEST_CASE("Tabela: as faixas dividem o intervalo em partes iguais") {

    // Campo de 800 de altura em 4 linhas: 200 cada.
    CHECK(Tabela::Espessura(800.0f, 4) == doctest::Approx(200.0f));
    CHECK(Tabela::Inicio(0.0f, 800.0f, 4, 0) == doctest::Approx(0.0f));
    CHECK(Tabela::Inicio(0.0f, 800.0f, 4, 1) == doctest::Approx(200.0f));
    CHECK(Tabela::Inicio(0.0f, 800.0f, 4, 3) == doctest::Approx(600.0f));
}

TEST_CASE("Tabela: o centro da faixa fica no meio dela") {

    CHECK(Tabela::Centro(0.0f, 800.0f, 4, 0) == doctest::Approx(100.0f));
    CHECK(Tabela::Centro(0.0f, 800.0f, 4, 3) == doctest::Approx(700.0f));
}

TEST_CASE("Tabela: a origem desloca tudo") {

    // O campo de jogo nao comeca em zero.
    CHECK(Tabela::Inicio(100.0f, 400.0f, 4, 0) == doctest::Approx(100.0f));
    CHECK(Tabela::Inicio(100.0f, 400.0f, 4, 2) == doctest::Approx(300.0f));
    CHECK(Tabela::Centro(100.0f, 400.0f, 4, 0) == doctest::Approx(150.0f));
}

TEST_CASE("Tabela: uma tabela de uma faixa centra a faixa no campo") {

    CHECK(Tabela::Centro(0.0f, 800.0f, 1, 0) == doctest::Approx(400.0f));
}

TEST_CASE("Tabela: forma invalida e recusada, nao corrigida em silencio") {

    CHECK(Tabela::Valida(Tabela::Forma{4, 5}));
    CHECK_FALSE(Tabela::Valida(Tabela::Forma{0, 5}));
    CHECK_FALSE(Tabela::Valida(Tabela::Forma{4, 0}));
    CHECK_FALSE(Tabela::Valida(Tabela::Forma{-1, 5}));
}

TEST_CASE("Tabela: zero faixas nao divide por zero") {

    CHECK(Tabela::Espessura(800.0f, 0) == doctest::Approx(800.0f));
}
