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

TEST_CASE("Tabela: FaixaDe e o inverso de Centro") {

    // A PROPRIEDADE QUE FAZ O TELEGRAFO SER HONESTO. O chefe pergunta "em que
    // linha esta o jogador" e manda varrer essa linha; a estrategia pergunta
    // "onde fica o centro da linha" e varre ali. Se as duas contas nao fossem
    // inversas, o ataque varreria uma faixa vizinha a que foi anunciada.
    for (int quantas = 1; quantas <= 8; ++quantas) {
        for (int i = 0; i < quantas; ++i) {
            const float centro = Tabela::Centro(0.0f, 775.0f, quantas, i);
            CAPTURE(quantas);
            CAPTURE(i);
            CHECK(Tabela::FaixaDe(0.0f, 775.0f, quantas, centro) == i);
        }
    }
}

TEST_CASE("Tabela: quem esta fora da tabela pertence a borda mais proxima") {

    // Nunca -1: o jogador meio pixel fora da borda tem de continuar pertencendo
    // a alguma linha, senao o chefe varreria uma linha inexistente e o ataque
    // sumiria - o jogador leria isso como o chefe travando.
    CHECK(Tabela::FaixaDe(0.0f, 800.0f, 4, -50.0f) == 0);
    CHECK(Tabela::FaixaDe(0.0f, 800.0f, 4, 0.0f) == 0);
    CHECK(Tabela::FaixaDe(0.0f, 800.0f, 4, 799.9f) == 3);
    CHECK(Tabela::FaixaDe(0.0f, 800.0f, 4, 800.0f) == 3);
    CHECK(Tabela::FaixaDe(0.0f, 800.0f, 4, 5000.0f) == 3);
}

TEST_CASE("Tabela: as faixas cobrem o intervalo inteiro, sem buraco nem sobra") {

    // Varre o campo pixel a pixel e exige que toda posicao caia em exatamente
    // uma faixa valida. Um buraco aqui seria um lugar onde o jogador existe mas
    // nenhuma consulta o alcanca - um ponto cego permanente no meio da tabela.
    constexpr float altura = 775.0f;
    constexpr int linhas = 4;

    for (int px = 0; px < static_cast<int>(altura); ++px) {
        const int faixa = Tabela::FaixaDe(0.0f, altura, linhas, static_cast<float>(px));
        CAPTURE(px);
        CHECK(faixa >= 0);
        CHECK(faixa < linhas);
    }
}

TEST_CASE("Tabela: uma tabela de uma faixa so poe tudo na faixa zero") {

    CHECK(Tabela::FaixaDe(0.0f, 800.0f, 1, 0.0f) == 0);
    CHECK(Tabela::FaixaDe(0.0f, 800.0f, 1, 799.0f) == 0);
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
    CHECK(Tabela::FaixaDe(0.0f, 800.0f, 0, 400.0f) == 0);
}
