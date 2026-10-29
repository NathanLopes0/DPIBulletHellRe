//
// A Promise do JavaScript: quando ela resolve.
//

#include "doctest.h"

#include "../Source/Attacks/Shared/Promessa.h"

TEST_CASE("Promessa: resolve quando o sorteio fica abaixo da fracao") {
    CHECK(Promessa::Resolve(0.1f, 0.6f));
    CHECK(Promessa::Resolve(0.59f, 0.6f));
    CHECK_FALSE(Promessa::Resolve(0.6f, 0.6f));
    CHECK_FALSE(Promessa::Resolve(0.9f, 0.6f));
}

TEST_CASE("Promessa: fracao zero nunca resolve, fracao um sempre resolve") {

    // AS DUAS PONTAS PRECISAM SER EXATAS. Com "todas resolvem" o designer
    // espera todas - inclusive a do sorteio mais alto que o gerador da.
    CHECK_FALSE(Promessa::Resolve(0.0f, 0.0f));
    CHECK_FALSE(Promessa::Resolve(0.5f, 0.0f));
    CHECK(Promessa::Resolve(0.0f, 1.0f));
    CHECK(Promessa::Resolve(0.9999f, 1.0f));

    // E O SORTEIO 1,0 TAMBEM. O jogo sorteia com uniform_real_distribution<float>,
    // que no papel nunca devolve o limite de cima - mas com float o arredondamento
    // pode devolve-lo (LWG 2524). Sem a guarda, "todas resolvem" falharia ai.
    CHECK(Promessa::Resolve(1.0f, 1.0f));
}
