//
// A batida de um botao.
//

#include "doctest.h"

#include "../Source/Entrada/Borda.h"

TEST_CASE("Borda: conta o aperto uma vez, e nao o tempo segurado") {
    Borda b;
    CHECK(b.Bateu(true));
    CHECK_FALSE(b.Bateu(true));    // segurado
    CHECK_FALSE(b.Bateu(false));
    CHECK(b.Bateu(true));          // soltou e apertou de novo
}

TEST_CASE("Borda: o botao que ja vinha apertado nao conta") {
    Borda b(true);
    CHECK_FALSE(b.Bateu(true));
    CHECK_FALSE(b.Bateu(false));
    CHECK(b.Bateu(true));
}

TEST_CASE("Borda: Segurar faz o que esta apertado agora nao contar") {
    Borda b;
    b.Bateu(false);
    b.Segurar();
    CHECK_FALSE(b.Bateu(true));
    CHECK(b.Bateu(false) == false);
    CHECK(b.Bateu(true));
}
