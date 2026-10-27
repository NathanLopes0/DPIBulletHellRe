//
// O ciclo do agente: pensa, age, observa - e o AGE uma vez por volta.
//

#include "doctest.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

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

TEST_CASE("ReAct: o Pensa de cada volta e sorteado em volta do do degrau, e o Age sai uma vez por volta") {
    for (const auto nivel : {Dificuldade::Nivel::Normal, Dificuldade::Nivel::Dificil}) {
        const ReAct::Degrau d = ReAct::DegrauDo(nivel);
        Variacao::Sorteio s(42u);
        float menor = 9.0f, maior = 0.0f;
        for (int i = 0; i < 2000; ++i) {
            const float p = ReAct::PensaDaVolta(s, d.pensa);
            REQUIRE(p >= 0.7f * d.pensa);
            REQUIRE(p < 1.3f * d.pensa);
            menor = std::min(menor, p);
            maior = std::max(maior, p);
        }
        CHECK(menor < 0.72f * d.pensa);
        CHECK(maior > 1.28f * d.pensa);

        // COMO O JULIO RODA: um Pensa novo a cada volta. O Age sai uma vez por
        // volta, sem pular nenhuma, quadro a quadro.
        ReAct::Ciclo c;
        c.observa = d.observa;
        c.pensa = ReAct::PensaDaVolta(s, d.pensa);
        int ages = 0, voltas = 0;
        for (int q = 0; q < 60 * 60; ++q) {
            const float antes = c.t;
            ages += ReAct::Avancar(c, 1.0f / 60.0f) ? 1 : 0;
            if (c.t < antes) {
                ++voltas;
                c.pensa = std::max(ReAct::PensaDaVolta(s, d.pensa), c.t + 0.01f);
            }
        }
        CHECK(std::abs(ages - voltas) <= 1);
        // UMA FERRAMENTA A CADA VOLTA DE (pensa + age + observa) EM MEDIA.
        const float volta = d.pensa + ReAct::kAge + d.observa;
        CHECK(static_cast<float>(voltas) == doctest::Approx(60.0f / volta).epsilon(0.08));
    }
}

TEST_CASE("ReAct: o degrau do Normal e o de sempre, e o do Dificil age mais vezes") {
    const ReAct::Degrau n = ReAct::DegrauDo(Dificuldade::Nivel::Normal);
    CHECK(n.pensa == ReAct::kPensa);
    CHECK(n.observa == ReAct::kObserva);
    const ReAct::Degrau d = ReAct::DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(d.pensa < n.pensa);
    CHECK(d.observa < n.observa);
    // O PLACAR AINDA SE LE: o Pensa mais curto do Dificil passa de meio segundo.
    CHECK(0.7f * d.pensa >= 0.5f);
}
