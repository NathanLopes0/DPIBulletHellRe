//
// A amostragem com temperatura: o softmax, o sorteio e os candidatos de angulo.
//

#include "doctest.h"

#include <cmath>
#include <numeric>

#include "../Source/Attacks/Temperatura.h"

TEST_CASE("Temperatura: as probabilidades somam um, e o mais provavel e o de maior logit") {
    const std::vector<float> logits = {0.0f, -1.0f, -2.0f};
    for (const float t : {0.2f, 0.8f, 1.6f, 10.0f}) {
        CAPTURE(t);
        const auto p = Temperatura::Probabilidades(logits, t);
        CHECK(std::accumulate(p.begin(), p.end(), 0.0f) == doctest::Approx(1.0f));
        CHECK(p[0] > p[1]);
        CHECK(p[1] > p[2]);
    }
}

TEST_CASE("Temperatura: fria escolhe, quente achata") {
    const std::vector<float> logits = {0.0f, -1.0f, -2.0f};
    const auto fria = Temperatura::Probabilidades(logits, 0.2f);
    const auto quente = Temperatura::Probabilidades(logits, 10.0f);
    CHECK(fria[0] > 0.99f);
    CHECK(quente[0] < 0.4f);
    CHECK(quente[2] > 0.28f);

    // Zero ou menos: a escolha gulosa, sem sorteio nenhum.
    const auto gulosa = Temperatura::Probabilidades(logits, 0.0f);
    CHECK(gulosa[0] == doctest::Approx(1.0f));
    CHECK(gulosa[1] == doctest::Approx(0.0f));

    // E logits enormes nao estouram a exponencial.
    const auto grande = Temperatura::Probabilidades({1000.0f, 999.0f}, 1.0f);
    CHECK(grande[0] == doctest::Approx(1.0f / (1.0f + std::exp(-1.0f))));
}

TEST_CASE("Temperatura: o sorteio cai no candidato da faixa de u, e o u de 1 nao escapa") {
    const std::vector<float> p = {0.5f, 0.3f, 0.2f};
    CHECK(Temperatura::Amostrar(p, 0.0f) == 0);
    CHECK(Temperatura::Amostrar(p, 0.49f) == 0);
    CHECK(Temperatura::Amostrar(p, 0.5f) == 1);
    CHECK(Temperatura::Amostrar(p, 0.79f) == 1);
    CHECK(Temperatura::Amostrar(p, 0.81f) == 2);
    CHECK(Temperatura::Amostrar(p, 1.0f) == 2);

    // Um candidato de probabilidade zero nunca sai, nem no limite.
    CHECK(Temperatura::Amostrar({0.7f, 0.3f, 0.0f}, 1.0f) == 1);
}

TEST_CASE("Temperatura: os candidatos de angulo vao de -abertura/2 a +abertura/2, e o do meio e o mais provavel") {
    const auto c = Temperatura::DeAngulo(9, 120.0f, 15.0f);
    REQUIRE(c.desvios.size() == 9);
    CHECK(c.desvios.front() == doctest::Approx(-60.0f));
    CHECK(c.desvios[4] == doctest::Approx(0.0f));
    CHECK(c.desvios.back() == doctest::Approx(60.0f));
    CHECK(c.logits[4] == doctest::Approx(0.0f));
    CHECK(c.logits[0] == doctest::Approx(-4.0f));
    CHECK(c.logits[3] == doctest::Approx(c.logits[5]));

    const auto um = Temperatura::DeAngulo(1, 120.0f);
    REQUIRE(um.desvios.size() == 1);
    CHECK(um.desvios[0] == doctest::Approx(0.0f));
}
