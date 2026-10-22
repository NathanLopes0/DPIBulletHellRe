//
// O ciclo do agente: pensa, age, observa - e o AGE uma vez por volta.
//

#include "doctest.h"

#include "../Source/Attacks/ReAct.h"

TEST_CASE("ReAct: o ciclo passa por pensa, age e observa, e volta") {
    ReAct::Ciclo c;   // 1,0 + 0,5 + 0,6
    CHECK(ReAct::PassoDe(c) == ReAct::Passo::Pensa);
    c.t = 1.2f;
    CHECK(ReAct::PassoDe(c) == ReAct::Passo::Age);
    c.t = 1.8f;
    CHECK(ReAct::PassoDe(c) == ReAct::Passo::Observa);
}

TEST_CASE("ReAct: o agente age uma vez por volta, quando entra no Age") {
    ReAct::Ciclo c;
    int agiu = 0;
    float quando = -1.0f, t = 0.0f;
    for (int k = 0; k < 6 * 60; ++k) {   // 6 s a 60 quadros por segundo
        t += 1.0f / 60.0f;
        if (ReAct::Avancar(c, 1.0f / 60.0f)) {
            ++agiu;
            if (quando < 0.0f) quando = t;
        }
    }
    CHECK(agiu == 3);                          // a volta e de 2,1 s
    CHECK(quando == doctest::Approx(1.0f).epsilon(0.02));
}

TEST_CASE("ReAct: um quadro que pula o Age inteiro ainda age") {
    ReAct::Ciclo c;
    c.t = 0.9f;
    CHECK(ReAct::Avancar(c, 1.0f));            // de 0,9 a 1,9: atravessou o Age
    CHECK(ReAct::PassoDe(c) == ReAct::Passo::Observa);

    // E um quadro que da a volta inteira e cai no Pensa da volta seguinte
    // tambem agiu.
    ReAct::Ciclo d;
    d.t = 1.8f;
    CHECK(ReAct::Avancar(d, 2.0f));            // de 1,8 a 3,8 = 1,7 da volta seguinte
}

TEST_CASE("ReAct: sem tempo nao anda, e um ciclo de duracao zero nao age") {
    ReAct::Ciclo c;
    CHECK_FALSE(ReAct::Avancar(c, 0.0f));
    ReAct::Ciclo zero{0.0f, 0.0f, 0.0f, 0.0f};
    CHECK_FALSE(ReAct::Avancar(zero, 1.0f));
}
