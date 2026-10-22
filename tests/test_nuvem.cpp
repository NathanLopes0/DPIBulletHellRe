//
// A nuvem do Vitor: processa e chove, uma vez por volta.
//

#include "doctest.h"

#include "../Source/Attacks/Nuvem.h"

TEST_CASE("Nuvem: processa, chove no fim do processamento, descansa e volta") {
    Nuvem::Ciclo c;   // 2,4 + 0,8
    CHECK(Nuvem::Processando(c));

    int choveu = 0;
    float primeira = -1.0f, t = 0.0f;
    for (int k = 0; k < 10 * 60; ++k) {   // 10 s a 60 quadros por segundo
        t += 1.0f / 60.0f;
        if (Nuvem::Avancar(c, 1.0f / 60.0f)) {
            ++choveu;
            if (primeira < 0.0f) primeira = t;
            CHECK_FALSE(Nuvem::Processando(c));   // chove e ja descansa
        }
    }
    CHECK(choveu == 3);                    // a volta e de 3,2 s: aos 2,4, 5,6 e 8,8
    CHECK(primeira == doctest::Approx(2.4f).epsilon(0.02));
}

TEST_CASE("Nuvem: um quadro que pula o fim do processamento ainda chove") {
    Nuvem::Ciclo c;
    c.t = 2.3f;
    CHECK(Nuvem::Avancar(c, 0.5f));
    Nuvem::Ciclo d;
    d.t = 3.0f;
    CHECK(Nuvem::Avancar(d, 3.0f));        // de 3,0 a 6,0: 2,8 da volta seguinte
}

TEST_CASE("Nuvem: sem tempo nao anda, e um ciclo de duracao zero nao chove") {
    Nuvem::Ciclo c;
    CHECK_FALSE(Nuvem::Avancar(c, 0.0f));
    Nuvem::Ciclo zero{0.0f, 0.0f, 0.0f};
    CHECK_FALSE(Nuvem::Avancar(zero, 1.0f));
}
