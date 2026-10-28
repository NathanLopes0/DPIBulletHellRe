//
// O alfabeto do Vladimir, e o que o intercambio faz com ele.
//

#include "doctest.h"

#include "../Source/Attacks/Alfabeto.h"

TEST_CASE("Alfabeto: a troca do intercambio e o homomorfismo h(a) = b, h(b) = a") {
    CHECK(Alfabeto::Trocar(Alfabeto::kA) == Alfabeto::kB);
    CHECK(Alfabeto::Trocar(Alfabeto::kB) == Alfabeto::kA);
    // Aplicada duas vezes, volta ao que era: e uma troca, e nao uma rotacao.
    CHECK(Alfabeto::Trocar(Alfabeto::Trocar(Alfabeto::kA)) == Alfabeto::kA);
    // Quem nao e letra fica: o S da gramatica, o estado do automato.
    CHECK(Alfabeto::Trocar("S") == "S");
    CHECK(Alfabeto::Trocar("") == "");
}

TEST_CASE("Alfabeto: a palavra troca letra por letra, e o resto fica") {
    CHECK(Alfabeto::TrocarPalavra("abbab") == "baaba");
    CHECK(Alfabeto::TrocarPalavra("") == "");
    CHECK(Alfabeto::TrocarPalavra("aSb") == "bSa");
    CHECK(Alfabeto::TrocarPalavra(Alfabeto::TrocarPalavra("aabba")) == "aabba");
}

TEST_CASE("Alfabeto: so troca a letra em voo - sem posto e que nao e enfeite") {
    CHECK(Alfabeto::Troca(Alfabeto::kA, "", false));
    CHECK(Alfabeto::Troca(Alfabeto::kB, "", false));
    CHECK_FALSE(Alfabeto::Troca(Alfabeto::kA, Alfabeto::kPostoDaFita, false));   // a casa da fita
    CHECK_FALSE(Alfabeto::Troca(Alfabeto::kB, "", true));                        // enfeite
    CHECK_FALSE(Alfabeto::Troca("S", "", false));                                // nao e letra
}
