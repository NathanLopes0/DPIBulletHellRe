//
// A janela ancorada: o passo da aproximacao.
//

#include "doctest.h"

#include "../Source/Attacks/Ancora.h"

TEST_CASE("Ancora: a janela se aproxima do lugar dela sem passar, e chega") {
    Vector2 p(0.0f, 0.0f);
    const Vector2 alvo(100.0f, 50.0f);
    float antes = (alvo - p).Length();
    for (int k = 0; k < 120; ++k) {
        p = Ancora::Aproximar(p, alvo, Ancora::kTaxa, 1.0f / 60.0f);
        const float agora = (alvo - p).Length();
        // Encolhe a cada quadro - ate a conta em float nao ter mais o que tirar.
        if (antes > 0.01f) CHECK(agora < antes);
        CHECK(agora <= antes + 1.0e-4f);
        antes = agora;
    }
    CHECK(antes < 0.1f);   // em 2 s sobra e^-16 do caminho
}

TEST_CASE("Ancora: o passo nao depende do quadro") {
    const Vector2 de(0.0f, 0.0f), alvo(200.0f, 0.0f);
    const Vector2 um = Ancora::Aproximar(de, alvo, 8.0f, 0.02f);
    const Vector2 dois = Ancora::Aproximar(Ancora::Aproximar(de, alvo, 8.0f, 0.01f), alvo, 8.0f, 0.01f);
    CHECK(um.x == doctest::Approx(dois.x));
    // E a fracao e a da formula: 1 - e^-0,16.
    CHECK(um.x == doctest::Approx(200.0f * (1.0f - 0.852143789f)));
}

TEST_CASE("Ancora: sem taxa ou sem tempo, nao anda") {
    const Vector2 de(10.0f, 20.0f), alvo(200.0f, 0.0f);
    CHECK(Ancora::Aproximar(de, alvo, 0.0f, 0.1f).x == doctest::Approx(10.0f));
    CHECK(Ancora::Aproximar(de, alvo, 8.0f, 0.0f).x == doctest::Approx(10.0f));
}
