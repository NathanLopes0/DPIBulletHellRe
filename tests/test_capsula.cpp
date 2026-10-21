//
// A capsula: o anel em que ela abre.
//

#include "doctest.h"

#include <cmath>

#include "../Source/Attacks/Capsula.h"

TEST_CASE("Capsula: abre num anel igualmente espacado, todas no mesmo modulo") {
    const auto v = Capsula::Abertura(12, 90.0f, 150.0f);
    REQUIRE(v.size() == 12);
    for (size_t i = 0; i < v.size(); ++i) {
        CAPTURE(i);
        CHECK(v[i].Length() == doctest::Approx(150.0f));
        const Vector2& a = v[i];
        const Vector2& b = v[(i + 1) % v.size()];
        const float entre = std::atan2(a.x * b.y - a.y * b.x, a.x * b.x + a.y * b.y);
        CHECK(entre == doctest::Approx(2.0f * 3.14159265f / 12.0f).epsilon(0.001));
    }
}

TEST_CASE("Capsula: a primeira letra segue para onde a capsula ia") {
    // Voando para baixo (90 graus), a primeira sai para baixo.
    const auto baixo = Capsula::Abertura(7, 90.0f, 200.0f);
    CHECK(baixo[0].x == doctest::Approx(0.0f).epsilon(0.001));
    CHECK(baixo[0].y == doctest::Approx(200.0f));

    // Voando na diagonal para a esquerda e para baixo (135), a primeira tambem.
    const auto diagonal = Capsula::Abertura(5, 135.0f, 100.0f);
    CHECK(diagonal[0].x == doctest::Approx(-70.71f).epsilon(0.001));
    CHECK(diagonal[0].y == doctest::Approx(70.71f).epsilon(0.001));
}

TEST_CASE("Capsula: uma capsula vazia nao abre em nada") {
    CHECK(Capsula::Abertura(0, 90.0f, 150.0f).empty());
    CHECK(Capsula::Abertura(-3, 90.0f, 150.0f).empty());
}
