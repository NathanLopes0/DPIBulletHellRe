// A tabela do Thiago. Camada pura.
//
// O que estes testes protegem e o acordo entre duas pessoas que precisam
// concordar sobre onde fica a linha 2: a estrategia, que posiciona a varredura,
// e o chefe, que decide qual linha varrer a partir de onde o jogador esta.
// Se as duas discordassem, o jogo telegrafaria uma linha e varreria outra.

#include "doctest.h"

#include "../Source/Tabela.h"

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

    // O campo de jogo nao comeca em zero: a barra da nota ocupa o rodape, e o
    // campo util e passado como (origem, tamanho). Se as faixas ignorassem a
    // origem, a tabela sairia deslocada da tela inteira.
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

    // Nao deveria chegar aqui (Valida recusa antes), mas uma divisao por zero
    // aqui viraria NaN na posicao de um projetil, e um projetil em NaN nunca
    // sai da tela nem volta para o pool.
    CHECK(Tabela::Espessura(800.0f, 0) == doctest::Approx(800.0f));
}
