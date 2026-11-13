//
// O limite dos efeitos sonoros (Source/Motor/LimiteDeSom.h).
//

#include "doctest.h"

#include "../Source/Motor/LimiteDeSom.h"

TEST_CASE("LimiteDeSom: o disparo nao empilha - duas copias no maximo, e com intervalo") {
    using namespace LimiteDeSom;
    Controle c;
    CHECK(c.PodeTocar("disparo", 1000, 0, kDisparo));
    // Logo depois, nao: o intervalo.
    CHECK_FALSE(c.PodeTocar("disparo", 1050, 1, kDisparo));
    CHECK(c.PodeTocar("disparo", 1110, 1, kDisparo));
    // Com duas tocando, nem depois do intervalo.
    CHECK_FALSE(c.PodeTocar("disparo", 1500, 2, kDisparo));
    CHECK(c.PodeTocar("disparo", 1500, 1, kDisparo));
    // A recusa nao conta como comeco: o intervalo vale desde o ultimo que tocou.
    CHECK_FALSE(c.PodeTocar("disparo", 1560, 1, kDisparo));
    CHECK(c.PodeTocar("disparo", 1610, 1, kDisparo));
}

TEST_CASE("LimiteDeSom: cada grupo conta o seu, e o relogio que volta nao trava") {
    using namespace LimiteDeSom;
    Controle c;
    CHECK(c.PodeTocar("disparo", 500, 0, kDisparo));
    CHECK(c.PodeTocar("tiro", 510, 0, kTiro));
    CHECK_FALSE(c.PodeTocar("tiro", 540, 0, kTiro));
    CHECK(c.PodeTocar("tiro", 570, 1, kTiro));
    CHECK(c.PodeTocar("disparo", 100, 0, kDisparo));
    // Um chefe que solta uma gota a cada 60 ms toca, no maximo, um disparo a cada 120 ms.
    Controle chuva;
    int tocou = 0;
    for (std::uint32_t t = 0; t < 1200; t += 60) if (chuva.PodeTocar("disparo", t, 0, kDisparo)) ++tocou;
    CHECK(tocou == 10);
}
