//
// A comunicacao minima da dupla: o sinal do Botao 3 e o "valeu" de quem foi coberto.
//

#include "doctest.h"

#include <set>

#include "../Source/Interface/Comunicacao.h"

using namespace Comunicacao;

TEST_CASE("Comunicacao: o primeiro sinal sai sempre, e o proximo so depois da espera") {
    CHECK(PodeSinalizar(0.0f, kNuncaSinalizou));
    CHECK(PodeSinalizar(42.0f, kNuncaSinalizou));
    CHECK_FALSE(PodeSinalizar(10.0f + kEsperaDoSinal * 0.5f, 10.0f));
    CHECK(PodeSinalizar(10.0f + kEsperaDoSinal, 10.0f));
}

TEST_CASE("Comunicacao: o anel do sinal abre e apaga, e o coracao sobe e apaga") {
    CHECK(RaioDoSinal(0.0f) < RaioDoSinal(kDuracaoDoSinal * 0.5f));
    CHECK(RaioDoSinal(kDuracaoDoSinal * 0.5f) < RaioDoSinal(kDuracaoDoSinal));
    CHECK(AlfaDoSinal(0.0f) == doctest::Approx(1.0f));
    CHECK(AlfaDoSinal(kDuracaoDoSinal) == doctest::Approx(0.0f));
    CHECK(AlfaDoSinal(kDuracaoDoSinal * 0.5f) > 0.0f);

    CHECK(SubidaDoValeu(0.0f) == doctest::Approx(0.0f));
    CHECK(SubidaDoValeu(kDuracaoDoValeu) > SubidaDoValeu(kDuracaoDoValeu * 0.5f));
    // Inteiro no comeco, para ser visto; apagado no fim.
    CHECK(AlfaDoValeu(0.1f) == doctest::Approx(1.0f));
    CHECK(AlfaDoValeu(kDuracaoDoValeu * 0.9f) < 1.0f);
    CHECK(AlfaDoValeu(kDuracaoDoValeu) == doctest::Approx(0.0f));
    // Ele nasce acima do "v" da mira (Battle::kAlturaDaMira, 44), e nao em cima dele.
    CHECK(kAlturaDoValeu - kPixelDoCoracao * kAlturaDoCoracao / 2.0f > 44.0f + 8.0f);
}

TEST_CASE("Comunicacao: o coracao e simetrico e cabe na grade dele") {
    std::set<std::pair<int, int>> pixels;
    for (const Faixa& f : Coracao()) {
        CHECK(f.linha >= 0);
        CHECK(f.linha < kAlturaDoCoracao);
        CHECK(f.de >= 0);
        CHECK(f.de <= f.ate);
        CHECK(f.ate < kLarguraDoCoracao);
        for (int x = f.de; x <= f.ate; ++x) pixels.insert({x, f.linha});
    }
    // Espelhado no meio: um coracao torto se le como mancha.
    for (const auto& [x, y] : pixels) CHECK(pixels.count({kLarguraDoCoracao - 1 - x, y}) == 1);
    // A ponta embaixo e um pixel so.
    int naUltima = 0;
    for (const auto& [x, y] : pixels) if (y == kAlturaDoCoracao - 1) ++naUltima;
    CHECK(naUltima == 1);
}
