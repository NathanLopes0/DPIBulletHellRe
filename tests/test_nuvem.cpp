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

TEST_CASE("Nuvem: no fork o original some quando a nuvem chega, e os dois aparecem quando ela abre") {
    const Nuvem::Fork f;   // 0,4 / 1,0 / 1,6
    CHECK(Nuvem::Visivel(f, 0.0f, false));
    CHECK(Nuvem::Visivel(f, 0.39f, false));
    CHECK_FALSE(Nuvem::Visivel(f, 0.4f, false));
    CHECK_FALSE(Nuvem::Visivel(f, 0.99f, false));
    CHECK(Nuvem::Visivel(f, 1.0f, false));
    CHECK(Nuvem::Visivel(f, 5.0f, false));

    // o outro corpo nasce dentro da nuvem: nao aparece antes de ela abrir
    CHECK_FALSE(Nuvem::Visivel(f, 0.0f, true));
    CHECK_FALSE(Nuvem::Visivel(f, 0.39f, true));
    CHECK_FALSE(Nuvem::Visivel(f, 0.99f, true));
    CHECK(Nuvem::Visivel(f, 1.0f, true));
}

TEST_CASE("Nuvem: a saida do fork anda de 0 a 1 entre abrir e sair, sem solavanco") {
    const Nuvem::Fork f;
    CHECK(Nuvem::Saida(f, 0.0f) == 0.0f);
    CHECK(Nuvem::Saida(f, 1.0f) == 0.0f);
    CHECK(Nuvem::Saida(f, 1.3f) == doctest::Approx(0.5f));
    CHECK(Nuvem::Saida(f, 1.6f) == 1.0f);
    CHECK(Nuvem::Saida(f, 9.0f) == 1.0f);

    float antes = 0.0f;
    for (float t = 1.0f; t <= 1.6f; t += 0.01f) {
        const float s = Nuvem::Saida(f, t);
        CHECK(s >= antes);
        antes = s;
    }

    // devagar nas pontas: o primeiro e o ultimo centesimo andam bem menos que
    // o centesimo do meio
    const float meio = Nuvem::Saida(f, 1.305f) - Nuvem::Saida(f, 1.295f);
    CHECK(Nuvem::Saida(f, 1.01f) - Nuvem::Saida(f, 1.0f) < meio / 10.0f);
    CHECK(Nuvem::Saida(f, 1.6f) - Nuvem::Saida(f, 1.59f) < meio / 10.0f);
}

TEST_CASE("Nuvem: um fork que abre e sai no mesmo instante pula para o lugar") {
    const Nuvem::Fork f{0.4f, 1.0f, 1.0f};
    CHECK(Nuvem::Saida(f, 0.99f) == 0.0f);
    CHECK(Nuvem::Saida(f, 1.01f) == 1.0f);
}
