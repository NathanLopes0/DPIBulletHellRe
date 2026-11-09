//
// A fase 2 do Thiago: os registros que voam.
//

#include "doctest.h"

#include "../Source/Attacks/Dml.h"

TEST_CASE("Dml: o arco sai de um ponto, chega no outro e sobe a altura pedida no meio") {
    const Vector2 de(100.0f, 140.0f), para(700.0f, 640.0f);
    const Vector2 inicio = Dml::NoArco(de, para, 120.0f, 0.0f);
    const Vector2 fim = Dml::NoArco(de, para, 120.0f, 1.0f);
    CHECK(inicio.x == doctest::Approx(de.x));
    CHECK(inicio.y == doctest::Approx(de.y));
    CHECK(fim.x == doctest::Approx(para.x));
    CHECK(fim.y == doctest::Approx(para.y));

    const Vector2 meio = Dml::NoArco(de, para, 120.0f, 0.5f);
    CHECK(meio.x == doctest::Approx(400.0f));
    CHECK(meio.y == doctest::Approx(390.0f - 120.0f));

    // Altura zero e a reta; fora de 0 a 1, para nas pontas.
    CHECK(Dml::NoArco(de, para, 0.0f, 0.25f).y == doctest::Approx(265.0f));
    CHECK(Dml::NoArco(de, para, 120.0f, -1.0f).x == doctest::Approx(de.x));
    CHECK(Dml::NoArco(de, para, 120.0f, 2.0f).y == doctest::Approx(para.y));
}

TEST_CASE("Dml: a subida do que sai do chao para cima") {
    CHECK(Dml::Subida(300.0f, 900.0f) == doctest::Approx(50.0f));
    CHECK(Dml::Subida(0.0f, 900.0f) == doctest::Approx(0.0f));
    CHECK(Dml::Subida(300.0f, 0.0f) == doctest::Approx(0.0f));
}

TEST_CASE("Dml: o degrau do Normal e o de sempre, e o do Dificil aperta tudo") {
    const Dml::Degrau n = Dml::DegrauDo(Dificuldade::Nivel::Normal);
    CHECK(n.avisoDoDelete == Dml::kAvisoDoDelete);
    CHECK(n.avisoDoUpdate == Dml::kAvisoDoUpdate);
    CHECK(n.duracaoDoArco == Dml::kDuracaoDoArco);
    CHECK(n.gravidade == Dml::kGravidade);
    CHECK(n.velocidadeDoChute == Dml::kVelocidadeDoChute);

    const Dml::Degrau d = Dml::DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(d.avisoDoDelete < n.avisoDoDelete);
    CHECK(d.avisoDoUpdate < n.avisoDoUpdate);
    CHECK(d.duracaoDoArco < n.duracaoDoArco);
    CHECK(d.gravidade > n.gravidade);
    CHECK(d.velocidadeDoChute > n.velocidadeDoChute);
}
