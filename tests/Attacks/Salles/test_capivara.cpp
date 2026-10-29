//
// A capivara que atravessa o campo do Salles.
//

#include "../../doctest.h"

#include <algorithm>

#include "../../../Source/Attacks/Salles/Capivara.h"

TEST_CASE("Capivara: ela vem em mais ou menos seis de cada dez fases, entre kDe e kAte segundos, dos dois lados") {
    int vieram = 0, daEsquerda = 0;
    float primeiro = 99.0f, ultimo = 0.0f;
    constexpr int kFases = 5000;
    for (std::uint32_t i = 0; i < kFases; ++i) {
        const auto v = Capivara::Sortear(i * 2654435761u);
        CHECK(v.quando >= Capivara::kDe);
        CHECK(v.quando <= Capivara::kAte);
        if (!v.vem) continue;
        ++vieram;
        if (v.daEsquerda) ++daEsquerda;
        primeiro = std::min(primeiro, v.quando);
        ultimo = std::max(ultimo, v.quando);
    }
    const float fracao = static_cast<float>(vieram) / kFases;
    CHECK(fracao == doctest::Approx(Capivara::kChance).epsilon(0.05));
    CHECK(static_cast<float>(daEsquerda) / static_cast<float>(vieram) == doctest::Approx(0.5f).epsilon(0.1));
    CHECK(primeiro < Capivara::kDe + 0.5f);   // a janela inteira e usada
    CHECK(ultimo > Capivara::kAte - 0.5f);
}

TEST_CASE("Capivara: a mesma semente da a mesma visita") {
    const auto a = Capivara::Sortear(424242u);
    const auto b = Capivara::Sortear(424242u);
    CHECK(a.vem == b.vem);
    CHECK(a.quando == b.quando);
    CHECK(a.daEsquerda == b.daEsquerda);
}
