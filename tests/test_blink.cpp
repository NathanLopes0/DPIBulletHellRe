//
// O Blink: a fase final do Ricardo.
//

#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <set>
#include <vector>
#include "../Source/Attacks/Fase.h"

#include "../Source/Attacks/Blink.h"

TEST_CASE("Blink: o placar e o sketch - le o delay em milissegundos") {
    CHECK(*Blink::LerDelay("delay(1000);") == doctest::Approx(1.0f));
    CHECK(*Blink::LerDelay("digitalWrite(13, HIGH); delay(500);") == doctest::Approx(0.5f));
    CHECK(*Blink::LerDelay("delay(250)") == doctest::Approx(0.25f));
    CHECK_FALSE(Blink::LerDelay("delay();"));
    CHECK_FALSE(Blink::LerDelay("delay(0);"));
    CHECK_FALSE(Blink::LerDelay("delay(0.5);"));      // o Arduino conta em ms inteiros
    CHECK_FALSE(Blink::LerDelay("delay(-100);"));
    CHECK_FALSE(Blink::LerDelay("delay(500;"));
    CHECK_FALSE(Blink::LerDelay("void loop()"));
}

TEST_CASE("Blink: os lados acendem um de cada vez, comecando pela esquerda") {
    CHECK(Blink::EsquerdaAcesa(0.0f, 1.0f));
    CHECK(Blink::EsquerdaAcesa(0.99f, 1.0f));
    CHECK_FALSE(Blink::EsquerdaAcesa(1.01f, 1.0f));
    CHECK(Blink::EsquerdaAcesa(2.01f, 1.0f));
    CHECK_FALSE(Blink::EsquerdaAcesa(0.3f, 0.25f));
}

TEST_CASE("Blink: troca a cada delay, e quanto menor o delay mais trocas") {
    for (const float delay : {1.0f, 0.5f, 0.25f}) {
        CAPTURE(delay);
        int trocas = 0;
        float antes = 0.0f;
        for (int q = 1; q <= 4 * 60; ++q) {     // 4 s
            const float agora = static_cast<float>(q) / 60.0f;
            if (Blink::Trocou(antes, agora, delay)) ++trocas;
            antes = agora;
        }
        CHECK(trocas == static_cast<int>(4.0f / delay));
    }
    CHECK_FALSE(Blink::Trocou(0.0f, 0.01f, 1.0f));   // o comeco nao e troca
    CHECK_FALSE(Blink::Trocou(0.5f, 0.5f, 1.0f));
    CHECK_FALSE(Blink::Trocou(0.2f, 0.9f, 0.0f));
}

TEST_CASE("Blink: o sketch da luta - o delay de partida sorteado, pela metade a cada etapa, e o placar e a regra") {
    std::set<int> partidas;
    for (std::uint32_t semente = 0; semente < 100; ++semente) {
        Variacao::Sorteio s(semente);
        const Blink::Plano p = Blink::Sortear(s);
        partidas.insert(p.delayInicial);
        for (const Dificuldade::Nivel n : Dificuldade::kNiveis) {
            const Blink::Degrau d = Blink::DegrauDo(n);
            float antes = 1.0e9f;
            for (int k = 0; k < 3; ++k) {
                const auto lido = Blink::LerDelay(Blink::Anuncio(k, p, d));
                REQUIRE(lido.has_value());
                CHECK(*lido < antes);
                if (k > 0) CHECK(*lido == doctest::Approx(antes / 2.0f).epsilon(0.01));
                antes = *lido;
            }
        }
    }
    CHECK(partidas == std::set<int>{800, 1000, 1200});
    const Blink::Plano sempre;
    CHECK(Blink::Anuncio(0, sempre, Blink::DegrauDo(Dificuldade::Nivel::Normal)) == "digitalWrite(13, HIGH); delay(1000);");
    CHECK(Blink::Anuncio(2, sempre, Blink::DegrauDo(Dificuldade::Nivel::Normal)) == "digitalWrite(13, HIGH); delay(250);");
    CHECK(Blink::Anuncio(0, sempre, Blink::DegrauDo(Dificuldade::Nivel::Dificil)) == "digitalWrite(13, HIGH); delay(800);");
}
