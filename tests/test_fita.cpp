//
// A fita da maquina de Turing: quando uma casa e reescrita.
//

#include "doctest.h"

#include "../Source/Attacks/Fita.h"

#include <vector>
#include <algorithm>
#include <cstdint>

// A PARTE QUE DA PARA TESTAR DE UM COMPORTAMENTO QUE PRECISA DE SDL.
//
// ReescreverBehavior olha a cena, os corpos do chefe e o projetil - nada disso
// entra na suite. O que entra e a decisao, e e nela que estao os dois erros que
// derrubariam a fase: reescrever a casa no quadro em que ela nasce, e nunca
// reescrever porque a cabeca so passa perto.

TEST_CASE("Fita: a casa recem-escrita nao e reescrita pela cabeca que a escreveu") {

    // A CASA NASCE EMBAIXO DA CABECA. Se a idade minima nao segurasse, toda
    // casa viraria "b" no mesmo quadro em que foi escrita e a fita nunca seria
    // vista - seria so mais uma rajada de perseguidores.
    const std::vector<float> cabeca = {500.0f};
    CHECK_FALSE(Fita::DeveReescrever(0.0f, 0.9f, cabeca, 500.0f, 24.0f));
    CHECK_FALSE(Fita::DeveReescrever(0.89f, 0.9f, cabeca, 500.0f, 24.0f));
    CHECK(Fita::DeveReescrever(0.9f, 0.9f, cabeca, 500.0f, 24.0f));
}

TEST_CASE("Fita: so o x da cabeca conta, e o raio e inclusivo") {

    CHECK(Fita::CabecaSobreACasa(500.0f, 524.0f, 24.0f));
    CHECK(Fita::CabecaSobreACasa(524.0f, 500.0f, 24.0f));
    CHECK_FALSE(Fita::CabecaSobreACasa(500.0f, 524.5f, 24.0f));

    // Longe no x, nada - por mais velha que a casa seja.
    const std::vector<float> longe = {300.0f};
    CHECK_FALSE(Fita::DeveReescrever(10.0f, 0.9f, longe, 500.0f, 24.0f));
}

TEST_CASE("Fita: qualquer corpo do chefe e uma cabeca, e sem corpo nada e reescrito") {

    // Dois corpos, so o segundo em cima da casa: basta ele.
    const std::vector<float> dois = {100.0f, 505.0f};
    CHECK(Fita::DeveReescrever(2.0f, 0.9f, dois, 500.0f, 24.0f));

    // Batalha sem chefe em campo (ou ja encerrada): a fita fica como esta.
    const std::vector<float> nenhum;
    CHECK_FALSE(Fita::DeveReescrever(2.0f, 0.9f, nenhum, 500.0f, 24.0f));
}

TEST_CASE("Fita: na ponta a cabeca le o branco, troca de estado e volta") {
    CHECK(Fita::Branco(true) == "\xCE\xB4(q\xE2\x82\x80, \xE2\x96\xA1) = (q\xE2\x82\x81, \xE2\x96\xA1, \xE2\x86\x90)");
    CHECK(Fita::Branco(false) == "\xCE\xB4(q\xE2\x82\x81, \xE2\x96\xA1) = (q\xE2\x82\x80, \xE2\x96\xA1, \xE2\x86\x92)");
}

TEST_CASE("Fita: o placar e a transicao, e a seta e o sentido da cabeca") {
    CHECK(Fita::Transicao(true) == "\xCE\xB4(q\xE2\x82\x80, a) = (q\xE2\x82\x80, b, \xE2\x86\x92)");
    CHECK(Fita::Transicao(false) == "\xCE\xB4(q\xE2\x82\x81, a) = (q\xE2\x82\x81, b, \xE2\x86\x90)");
}

TEST_CASE("Fita: a pausa nas pontas e a primeira casa mudam de luta para luta, em volta das de sempre") {
    float soma = 0.0f, menorPausa = 9.0f, maiorPausa = 0.0f, menorEscrita = 9.0f, maiorEscrita = -1.0f;
    constexpr int kLutas = 2000;
    for (std::uint32_t semente = 0; semente < static_cast<std::uint32_t>(kLutas); ++semente) {
        Variacao::Sorteio s = Variacao::SorteioDa(semente, Fita::kFaseDaFita);
        const Fita::Plano p = Fita::Sortear(s);
        REQUIRE(p.pausaNaPonta >= 1.0f);
        REQUIRE(p.pausaNaPonta < 1.4f);
        REQUIRE(p.primeiraEscrita >= 0.0f);
        REQUIRE(p.primeiraEscrita < Fita::kEscrita);
        soma += p.pausaNaPonta;
        menorPausa = std::min(menorPausa, p.pausaNaPonta);
        maiorPausa = std::max(maiorPausa, p.pausaNaPonta);
        menorEscrita = std::min(menorEscrita, p.primeiraEscrita);
        maiorEscrita = std::max(maiorEscrita, p.primeiraEscrita);
    }
    // NA MEDIA, A PAUSA DE SEMPRE: e ela que deixa o aluno acertar o professor.
    CHECK(soma / kLutas == doctest::Approx(Fita::kPausaNaPonta).epsilon(0.02));
    CHECK(menorPausa < 1.02f);
    CHECK(maiorPausa > 1.38f);
    CHECK(menorEscrita < 0.01f);
    CHECK(maiorEscrita > Fita::kEscrita - 0.01f);
    const Fita::Plano sempre;
    CHECK(sempre.pausaNaPonta == Fita::kPausaNaPonta);
    CHECK(sempre.primeiraEscrita == 0.0f);
}

TEST_CASE("Fita: o degrau do Normal e o de sempre, e o do Dificil escreve mais casas") {
    CHECK(Fita::DegrauDo(Dificuldade::Nivel::Normal).escrita == Fita::kEscrita);
    CHECK(Fita::DegrauDo(Dificuldade::Nivel::Dificil).escrita < Fita::kEscrita);
    CHECK(Fita::DegrauDo(Dificuldade::Nivel::Dificil).escrita > 0.0f);
}
