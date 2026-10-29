//
// O contorno de quem o tiro do aluno destroi.
//

#include "doctest.h"

#include "../Source/Gameplay/Destrutivel.h"

using namespace Destrutivel;

TEST_CASE("Destrutivel: so o derrubavel sem blindagem, abrigo ou enfeite tem contorno") {
    CHECK(Pode(true, false, false, false));
    CHECK_FALSE(Pode(false, false, false, false));   // a bala comum: o tiro passa
    CHECK_FALSE(Pode(true, true, false, false));     // blindado: o tiro so some
    CHECK_FALSE(Pode(true, false, true, false));     // abrigo: come o tiro e fica
    CHECK_FALSE(Pode(true, false, false, true));     // enfeite: o tiro o atravessa
}

TEST_CASE("Destrutivel: a silhueta e opaca ou nada, com o limiar no meio da borda") {
    CHECK(AlfaDaSilhueta(0) == 0);
    CHECK(AlfaDaSilhueta(kLimiarDaSilhueta - 1) == 0);
    CHECK(AlfaDaSilhueta(kLimiarDaSilhueta) == 255);
    CHECK(AlfaDaSilhueta(255) == 255);
}
