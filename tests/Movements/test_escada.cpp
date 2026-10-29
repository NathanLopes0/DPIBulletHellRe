//
// A escada do modelo cascata: onde fica cada degrau, e quando se desce.
//

#include "../doctest.h"

#include "../../Source/Movements/Escada.h"

TEST_CASE("Escada: os degraus descem da esquerda para a direita, de margem a margem") {
    Escada::Geometria g;
    g.largura = 1200.0f; g.margem = 170.0f; g.topo = 110.0f; g.passo = 55.0f;

    const Vector2 primeiro = Escada::Degrau(0, 5, g);
    CHECK(primeiro.x == doctest::Approx(170.0f));
    CHECK(primeiro.y == doctest::Approx(110.0f));

    const Vector2 ultimo = Escada::Degrau(4, 5, g);
    CHECK(ultimo.x == doctest::Approx(1030.0f));
    CHECK(ultimo.y == doctest::Approx(330.0f));

    const Vector2 meio = Escada::Degrau(2, 5, g);
    CHECK(meio.x == doctest::Approx(600.0f));
    CHECK(meio.y == doctest::Approx(220.0f));

    // Fora da escada, o degrau mais perto.
    CHECK(Escada::Degrau(-1, 5, g).x == doctest::Approx(170.0f));
    CHECK(Escada::Degrau(9, 5, g).y == doctest::Approx(330.0f));
}

TEST_CASE("Escada: um degrau por etapa, e no ultimo depois do fim") {
    CHECK(Escada::DegrauNoTempo(0.0f, 3.4f, 5) == 0);
    CHECK(Escada::DegrauNoTempo(3.39f, 3.4f, 5) == 0);
    CHECK(Escada::DegrauNoTempo(3.4f, 3.4f, 5) == 1);
    CHECK(Escada::DegrauNoTempo(13.7f, 3.4f, 5) == 4);
    CHECK(Escada::DegrauNoTempo(40.0f, 3.4f, 5) == 4);
    CHECK(Escada::DegrauNoTempo(-1.0f, 3.4f, 5) == 0);
    CHECK(Escada::DegrauNoTempo(5.0f, 0.0f, 5) == 0);
}
