//
// A serpente: o rastro da cabeca, o giro dela e os rotulos em romano.
//

#include "doctest.h"

#include <cmath>

#include "../Source/Attacks/Rastro.h"

namespace {
    Vector2 V(const float x, const float y) { return Vector2(x, y); }
}

TEST_CASE("Rastro: o gomo esta no ponto do caminho a sua distancia da cabeca") {
    Rastro::Caminho c;
    c.Marcar(V(0, 0));
    c.Marcar(V(100, 0));
    c.Marcar(V(100, 100));      // dobrou para baixo
    CHECK(c.Comprimento() == doctest::Approx(200.0f));

    const Vector2 cabeca = c.Atras(0.0f);
    CHECK(cabeca.x == doctest::Approx(100.0f));
    CHECK(cabeca.y == doctest::Approx(100.0f));

    const Vector2 meio = c.Atras(50.0f);          // no meio do trecho vertical
    CHECK(meio.x == doctest::Approx(100.0f));
    CHECK(meio.y == doctest::Approx(50.0f));

    const Vector2 antes = c.Atras(150.0f);        // no trecho horizontal
    CHECK(antes.x == doctest::Approx(50.0f));
    CHECK(antes.y == doctest::Approx(0.0f));
}

TEST_CASE("Rastro: atras do comeco, o gomo ainda esta onde a serpente nasceu") {
    Rastro::Caminho c;
    c.Marcar(V(10, 20));
    c.Marcar(V(30, 20));
    const Vector2 p = c.Atras(500.0f);
    CHECK(p.x == doctest::Approx(10.0f));
    CHECK(p.y == doctest::Approx(20.0f));

    Rastro::Caminho vazio;
    CHECK(vazio.Vazio());
    CHECK(vazio.Atras(10.0f).x == doctest::Approx(0.0f));
}

TEST_CASE("Rastro: o salto nao soma comprimento, e o gomo salta quando chega nele") {
    // O GOTO: a cabeca anda 100 para a direita, salta para (500, 300) e anda
    // mais 50 para baixo.
    Rastro::Caminho c;
    c.Marcar(V(0, 0));
    c.Marcar(V(100, 0));
    c.Marcar(V(500, 300), true);
    c.Marcar(V(500, 350));
    CHECK(c.Comprimento() == doctest::Approx(150.0f));

    // A 30 px da cabeca, o gomo ja passou do salto: esta do lado de la.
    const Vector2 depois = c.Atras(30.0f);
    CHECK(depois.x == doctest::Approx(500.0f));
    CHECK(depois.y == doctest::Approx(320.0f));

    // A 80 px, ainda nao chegou nele: esta do lado de ca, e nunca no meio.
    const Vector2 antes = c.Atras(80.0f);
    CHECK(antes.x == doctest::Approx(70.0f));
    CHECK(antes.y == doctest::Approx(0.0f));

    // Exatamente no ponto do salto, ja esta do lado de la.
    const Vector2 no = c.Atras(50.0f);
    CHECK(no.x == doctest::Approx(500.0f));
    CHECK(no.y == doctest::Approx(300.0f));
}

TEST_CASE("Rastro: a direcao do gomo e a do trecho em que ele esta, e nunca a do salto") {
    Rastro::Caminho c;
    c.Marcar(V(0, 0));
    c.Marcar(V(100, 0));
    c.Marcar(V(500, 300), true);
    c.Marcar(V(500, 350));

    const Vector2 la = c.DirecaoAtras(30.0f);     // descendo
    CHECK(la.x == doctest::Approx(0.0f));
    CHECK(la.y == doctest::Approx(1.0f));

    const Vector2 ca = c.DirecaoAtras(80.0f);     // indo para a direita
    CHECK(ca.x == doctest::Approx(1.0f));
    CHECK(ca.y == doctest::Approx(0.0f));
}

TEST_CASE("Rastro: a cabeca vira no maximo o giro do quadro, e para quando aponta") {
    const Vector2 direita = V(1, 0);
    const Vector2 baixo = V(0, 1);

    const Vector2 um = Rastro::Girar(direita, baixo, 0.1f);
    CHECK(std::atan2(um.y, um.x) == doctest::Approx(0.1f));
    CHECK(um.Length() == doctest::Approx(1.0f));

    const Vector2 todo = Rastro::Girar(direita, baixo, 3.0f);   // giro de sobra
    CHECK(todo.x == doctest::Approx(0.0f).epsilon(0.001));
    CHECK(todo.y == doctest::Approx(1.0f));

    // Pelo lado mais curto: de 170 graus para -170 sao 20 graus, e nao 340.
    const float a = 170.0f * 3.14159265f / 180.0f, b = -170.0f * 3.14159265f / 180.0f;
    const Vector2 curto = Rastro::Girar(V(std::cos(a), std::sin(a)), V(std::cos(b), std::sin(b)), 0.05f);
    float novo = std::atan2(curto.y, curto.x);
    if (novo < 0.0f) novo += 2.0f * 3.14159265f;
    CHECK(novo == doctest::Approx(a + 0.05f));
}

TEST_CASE("Rastro: os rotulos em algarismos romanos") {
    CHECK(Rastro::Romano(1) == "I");
    CHECK(Rastro::Romano(2) == "II");
    CHECK(Rastro::Romano(4) == "IV");
    CHECK(Rastro::Romano(9) == "IX");
    CHECK(Rastro::Romano(14) == "XIV");
    CHECK(Rastro::Romano(40) == "XL");
    CHECK(Rastro::Romano(1999) == "MCMXCIX");
    CHECK(Rastro::Romano(3999) == "MMMCMXCIX");
    CHECK(Rastro::Romano(0).empty());
    CHECK(Rastro::Romano(4000).empty());
}
