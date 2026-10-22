//
// O Blink: a fase final do Ricardo.
//

#include "doctest.h"

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
