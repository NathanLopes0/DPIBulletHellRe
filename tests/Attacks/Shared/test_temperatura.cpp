//
// A amostragem com temperatura: o softmax, o sorteio e os candidatos de angulo.
//

#include "../../doctest.h"

#include <cmath>
#include <numeric>
#include <cstdint>
#include <set>

#include "../../../Source/Attacks/Shared/Temperatura.h"

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

TEST_CASE("Temperatura: cada luta sorteia a temperatura de cada etapa na faixa dela, em decimos") {
    std::set<int> fria, morna, quente;
    for (std::uint32_t semente = 0; semente < 300; ++semente) {
        Variacao::Sorteio s = Variacao::SorteioDa(semente, "StateOne");
        const auto t = Temperatura::Sortear(s);
        const int f = static_cast<int>(std::lround(t[0] * 10.0f));
        const int m = static_cast<int>(std::lround(t[1] * 10.0f));
        const int q = static_cast<int>(std::lround(t[2] * 10.0f));
        CHECK(std::fabs(t[0] * 10.0f - static_cast<float>(f)) < 1e-4f);   // decimos exatos
        CHECK(f >= 1);
        CHECK(f <= 3);
        CHECK(m >= 6);
        CHECK(m <= 10);
        CHECK(q >= 14);
        CHECK(q <= 18);
        fria.insert(f);
        morna.insert(m);
        quente.insert(q);
    }
    CHECK(fria.size() == 3u);
    CHECK(morna.size() == 5u);
    CHECK(quente.size() == 5u);
}

TEST_CASE("Temperatura: o placar da etapa diz o numero, e o de sempre e o do arquivo") {
    // OS TEXTOS DE julio/fases.json, que o placar mostrava antes do sorteio.
    CHECK(Temperatura::Anuncio(Temperatura::kDeSempre[0], false) == "temperatura = 0,2");
    CHECK(Temperatura::Anuncio(Temperatura::kDeSempre[1], false) == "temperatura = 0,8");
    CHECK(Temperatura::Anuncio(Temperatura::kDeSempre[2], true) == "temperatura = 1,6 \xc2\xb7 alucinando");
    CHECK(Temperatura::Anuncio(1.0f, false) == "temperatura = 1,0");
    CHECK(Temperatura::Anuncio(0.1f, false) == "temperatura = 0,1");
}

TEST_CASE("Temperatura: no Dificil cada etapa roda mais quente, e o Normal e o de antes") {
    for (std::uint32_t semente = 0; semente < 300; ++semente) {
        Variacao::Sorteio a = Variacao::SorteioDa(semente, "StateOne");
        Variacao::Sorteio b = Variacao::SorteioDa(semente, "StateOne");
        Variacao::Sorteio c = Variacao::SorteioDa(semente, "StateOne");
        CHECK(Temperatura::Sortear(a) == Temperatura::Sortear(b, Dificuldade::Nivel::Normal));
        const auto d = Temperatura::Sortear(c, Dificuldade::Nivel::Dificil);
        CHECK(d[0] >= 0.3f - 1e-4f);
        CHECK(d[0] <= 0.5f + 1e-4f);
        CHECK(d[1] >= 0.9f - 1e-4f);
        CHECK(d[1] <= 1.3f + 1e-4f);
        CHECK(d[2] >= 1.8f - 1e-4f);
        CHECK(d[2] <= 2.2f + 1e-4f);
        // AS ETAPAS AINDA SOBEM: a fria do Dificil e mais fria que a morna, e
        // a morna que a quente.
        CHECK(d[0] < d[1]);
        CHECK(d[1] < d[2]);
    }
    CHECK(Temperatura::kDoLoopNoDificil > 0.3f);
}
