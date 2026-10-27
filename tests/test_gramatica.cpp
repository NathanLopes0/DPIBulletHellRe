//
// A gramatica livre de contexto da fase 2 do Vladimir.
//

#include "doctest.h"

#include <string>

#include "../Source/Attacks/Gramatica.h"

TEST_CASE("Gramatica: derivar n vezes e esvaziar escreve exatamente a^n b^n") {
    for (int n = 0; n <= 6; ++n) {
        CAPTURE(n);
        std::string forma = "S";
        for (int i = 0; i < n; ++i) forma = Gramatica::Derivar(forma);
        CHECK(forma == std::string(static_cast<size_t>(n), 'a') + "S" + std::string(static_cast<size_t>(n), 'b'));
        CHECK(Gramatica::Esvaziar(forma) == Gramatica::Palavra(n));
    }
    // Sem variavel, nada a derivar nem a esvaziar.
    CHECK(Gramatica::Derivar("aabb") == "aabb");
    CHECK(Gramatica::Esvaziar("aabb") == "aabb");
}

TEST_CASE("Gramatica: o placar e um passo da derivacao, e a vazia e epsilon") {
    CHECK(Gramatica::Passo("aSb", "aaSbb") == "aSb \xE2\x86\x92 aaSbb");
    CHECK(Gramatica::Passo("S", "") == "S \xE2\x86\x92 \xCE\xB5");
}

TEST_CASE("Gramatica: o S deriva de duas a quatro vezes, todas possiveis") {
    bool viu[Gramatica::kMaisDerivacoes + 1] = {};
    for (std::uint32_t s = 0; s < 300; ++s) {
        const int n = Gramatica::SortearDerivacoes(s * 2654435761u);
        CAPTURE(n);
        REQUIRE(n >= Gramatica::kMenosDerivacoes);
        REQUIRE(n <= Gramatica::kMaisDerivacoes);
        viu[n] = true;
    }
    for (int n = Gramatica::kMenosDerivacoes; n <= Gramatica::kMaisDerivacoes; ++n) CHECK(viu[n]);
}

TEST_CASE("Gramatica: o degrau do Normal e o de sempre, e o do Dificil deriva mais depressa") {
    const Gramatica::Degrau n = Gramatica::DegrauDo(Dificuldade::Nivel::Normal);
    CHECK(n.derivar == Gramatica::kDerivar);
    const Gramatica::Degrau d = Gramatica::DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(d.derivar < n.derivar);
    CHECK(d.derivar > 0.0f);
}
