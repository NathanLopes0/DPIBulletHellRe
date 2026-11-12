//
// O fundo do campo da batalha.
//

#include "doctest.h"

#include <algorithm>
#include <cstdint>

#include "../Source/Interface/FundoDoCampo.h"

using namespace FundoDoCampo;

namespace {
    int Brilho(const std::uint32_t p) {
        return static_cast<int>((p >> 16) & 0xFF) + static_cast<int>((p >> 8) & 0xFF) + static_cast<int>(p & 0xFF);
    }
}

TEST_CASE("Fundo do campo: a grade cai de 40 em 40 pixels do jogo, mais forte de 200 em 200") {
    CHECK(LinhaDaGrade(0, 1.0f) == 2);
    CHECK(LinhaDaGrade(40, 1.0f) == 1);
    CHECK(LinhaDaGrade(200, 1.0f) == 2);
    CHECK(LinhaDaGrade(20, 1.0f) == 0);
    CHECK(LinhaDaGrade(41, 1.0f) == 0);

    // Na tela cheia, no pixel da tela correspondente.
    CHECK(LinhaDaGrade(54, 1.35f) == 1);
    CHECK(LinhaDaGrade(270, 1.35f) == 2);
    CHECK(LinhaDaGrade(55, 1.35f) == 0);
    CHECK(LinhaDaGrade(-40, 1.0f) == 0);
}

TEST_CASE("Fundo do campo: cada linha da grade tem um pixel da tela, em qualquer escala") {
    for (const float escala : {1.0f, 1.35f, 2.0f, 2.7f}) {
        CAPTURE(escala);
        int linhas = 0;
        const int largura = static_cast<int>(1200.0f * escala);
        for (int p = 0; p < largura; ++p) {
            if (LinhaDaGrade(p, escala) == 0) continue;
            ++linhas;
            CHECK(LinhaDaGrade(p + 1, escala) == 0);
        }
        CHECK(linhas == 30);
    }
}

TEST_CASE("Fundo do campo: nenhum pixel e preto, e o alto e mais claro que o pe") {
    const int largura = 240, altura = 160;
    const auto pixels = Pintar(largura, altura, 1.0f);
    REQUIRE(pixels.size() == static_cast<size_t>(largura * altura));

    for (const std::uint32_t p : pixels) {
        CHECK((p >> 24) == 0xFFu);
        CHECK(Brilho(p) >= 20);
    }

    // Longe das linhas da grade: o meio de um quadrado, em cima e embaixo.
    const auto pixel = [&](const int x, const int y) { return pixels[static_cast<size_t>(y * largura + x)]; };
    CHECK(Brilho(pixel(100, 20)) > Brilho(pixel(100, 140)));

    // A linha da grade se destaca do fundo ao lado dela, e o meio e mais claro que a lateral.
    CHECK(Brilho(pixel(100, 40)) > Brilho(pixel(100, 39)));
    CHECK(Brilho(pixel(100, 20)) > Brilho(pixel(5, 20)));
}

TEST_CASE("Fundo do campo: tamanho invalido nao pinta nada") {
    CHECK(Pintar(0, 100, 1.0f).empty());
    CHECK(Pintar(100, -1, 1.0f).empty());
}
