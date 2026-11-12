//
// O fundo do campo da batalha: um caderno quadriculado no escuro.
//

#pragma once

#include <cstdint>
#include <vector>

namespace FundoDoCampo {

    struct Rgb {
        std::uint8_t r, g, b;
    };

    /// O degrade, de cima para baixo: escuro, para os tiros continuarem saltando aos olhos.
    constexpr Rgb kTopo{22, 27, 46};
    constexpr Rgb kBase{9, 11, 20};

    /// O quadriculado, em pixels do jogo: uma linha a cada kEntreLinhas, e uma mais forte a cada
    /// kForteACada (a de 200 em 200, que cai no meio da tela).
    constexpr float kEntreLinhas = 40.0f;
    constexpr int kForteACada = 5;
    constexpr Rgb kCorDaGrade{110, 130, 190};
    constexpr float kLinhaFraca = 0.08f;
    constexpr float kLinhaForte = 0.15f;

    /// Quanto as laterais escurecem em relacao ao meio.
    constexpr float kVinheta = 0.3f;

    /// Se o pixel 'p' da tela (coluna ou linha) cai numa linha da grade: 0 nao, 1 fraca, 2 forte.
    /// A linha tem um pixel da tela em qualquer escala.
    int LinhaDaGrade(int p, float escala);

    /// O fundo em 'largura' x 'altura' pixels da tela, com 'escala' pixels da tela por pixel do jogo:
    /// ARGB8888, linha por linha, opaco.
    std::vector<std::uint32_t> Pintar(int largura, int altura, float escala);
}
