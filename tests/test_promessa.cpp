//
// A Promise do JavaScript: quando ela resolve.
//

#include "doctest.h"

#include "../Source/Attacks/Promessa.h"

TEST_CASE("Promessa: resolve quando o sorteio fica abaixo da fracao") {
    CHECK(Promessa::Resolve(0.1f, 0.6f));
    CHECK(Promessa::Resolve(0.59f, 0.6f));
    CHECK_FALSE(Promessa::Resolve(0.6f, 0.6f));
    CHECK_FALSE(Promessa::Resolve(0.9f, 0.6f));
}

TEST_CASE("Promessa: fracao zero nunca resolve, fracao um sempre resolve") {

    // As duas pontas precisam ser exatas.
    CHECK_FALSE(Promessa::Resolve(0.0f, 0.0f));
    CHECK_FALSE(Promessa::Resolve(0.5f, 0.0f));
    CHECK(Promessa::Resolve(0.0f, 1.0f));
    CHECK(Promessa::Resolve(0.9999f, 1.0f));

    // E o sorteio 1,0 tambem.
    CHECK(Promessa::Resolve(1.0f, 1.0f));
}
