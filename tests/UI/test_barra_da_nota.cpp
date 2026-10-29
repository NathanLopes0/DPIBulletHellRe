//
// A barra da nota e a tela de carregamento: o que elas mostram a cada quadro.
//

#include "../doctest.h"

#include "../../Source/UI/BarraDaNota.h"
#include "../../Source/UI/Carregamento.h"

TEST_CASE("BarraDaNota: o preenchimento corre atras da nota e chega nela") {
    BarraDaNota::Estado e = BarraDaNota::Comecar(40.0f);
    CHECK(e.mostrada == doctest::Approx(40.0f));
    BarraDaNota::Avancar(e, 50.0f, 0.016f);
    CHECK(e.mostrada > 40.0f);
    CHECK(e.mostrada < 50.0f);            // nao salta
    CHECK(e.brilho > 0.9f);               // subiu: o clarao
    for (int i = 0; i < 120; ++i) BarraDaNota::Avancar(e, 50.0f, 0.016f);
    CHECK(e.mostrada == doctest::Approx(50.0f));
    CHECK(e.brilho == doctest::Approx(0.0f));
}

TEST_CASE("BarraDaNota: a queda deixa um rastro parado, que depois encolhe ate a nota") {
    BarraDaNota::Estado e = BarraDaNota::Comecar(70.0f);
    BarraDaNota::Avancar(e, 62.0f, 0.016f);
    CHECK(e.rastro == doctest::Approx(70.0f));
    CHECK(e.tremor > 0.9f);
    // PARADO no comeco...
    for (int i = 0; i < 10; ++i) BarraDaNota::Avancar(e, 62.0f, 0.016f);
    CHECK(e.rastro == doctest::Approx(70.0f));
    // ...e depois some.
    for (int i = 0; i < 120; ++i) BarraDaNota::Avancar(e, 62.0f, 0.016f);
    CHECK(e.rastro == doctest::Approx(62.0f));
    CHECK(e.tremor == doctest::Approx(0.0f));
    // O RASTRO NUNCA FICA ATRAS do preenchimento.
    BarraDaNota::Avancar(e, 80.0f, 0.016f);
    CHECK(e.rastro >= e.mostrada);
}

TEST_CASE("BarraDaNota: duas quedas seguidas guardam o rastro da primeira, e a queda pequena nao sacode") {
    BarraDaNota::Estado e = BarraDaNota::Comecar(80.0f);
    BarraDaNota::Avancar(e, 75.0f, 0.016f);
    BarraDaNota::Avancar(e, 70.0f, 0.016f);
    CHECK(e.rastro == doctest::Approx(80.0f));
    BarraDaNota::Estado f = BarraDaNota::Comecar(80.0f);
    BarraDaNota::Avancar(f, 79.9f, 0.016f);
    CHECK(f.tremor == doctest::Approx(0.0f));
    CHECK(BarraDaNota::X(50.0f, 1000) == doctest::Approx(500.0f));
    CHECK(BarraDaNota::X(150.0f, 1000) == doctest::Approx(1000.0f));
    CHECK(BarraDaNota::X(-5.0f, 1000) == doctest::Approx(0.0f));
}

TEST_CASE("Carregamento: a barra nao enche num piscar, segura cheia, e o veu some depois") {
    Carregamento::Estado e;
    // O CARREGAMENTO DE VERDADE acaba de uma vez...
    Carregamento::Avancar(e, 1.0f, 0.016f);
    CHECK(e.mostrado < 0.1f);             // ...mas a barra anda no ritmo dela
    CHECK_FALSE(Carregamento::Pronto(e));
    CHECK(Carregamento::Opacidade(e) == doctest::Approx(1.0f));
    float t = 0.016f;
    while (!Carregamento::Pronto(e) && t < 5.0f) { Carregamento::Avancar(e, 1.0f, 0.016f); t += 0.016f; }
    CHECK(t >= 0.7f + Carregamento::kSegura - 0.05f);   // vazia a cheia, e cheia um instante
    CHECK(Carregamento::Opacidade(e) == doctest::Approx(1.0f).epsilon(0.1));
    CHECK_FALSE(Carregamento::Acabou(e));
    while (!Carregamento::Acabou(e) && t < 5.0f) { Carregamento::Avancar(e, 1.0f, 0.016f); t += 0.016f; }
    CHECK(Carregamento::Opacidade(e) == doctest::Approx(0.0f));
}

TEST_CASE("Carregamento: a barra nunca passa do progresso de verdade") {
    Carregamento::Estado e;
    for (int i = 0; i < 200; ++i) Carregamento::Avancar(e, 0.4f, 0.016f);
    CHECK(e.mostrado == doctest::Approx(0.4f));
    CHECK_FALSE(Carregamento::Pronto(e));
    CHECK(e.cheia < 0.0f);
}
