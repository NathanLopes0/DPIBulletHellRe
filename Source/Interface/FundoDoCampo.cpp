//
// O fundo do campo da batalha.
//

#include "FundoDoCampo.h"

#include <algorithm>
#include <cmath>

namespace FundoDoCampo {

namespace {
    float Canal(const std::uint8_t a, const std::uint8_t b, const float t) {
        return static_cast<float>(a) + (static_cast<float>(b) - static_cast<float>(a)) * t;
    }
}

int LinhaDaGrade(const int p, const float escala) {
    if (p < 0 || escala <= 0.0f) return 0;
    const float passo = kEntreLinhas * escala;
    const long k = std::lround(static_cast<float>(p) / passo);
    if (std::lround(static_cast<float>(k) * passo) != p) return 0;
    return k % kForteACada == 0 ? 2 : 1;
}

std::vector<std::uint32_t> Pintar(const int largura, const int altura, const float escala) {
    std::vector<std::uint32_t> pixels;
    if (largura <= 0 || altura <= 0) return pixels;
    pixels.resize(static_cast<size_t>(largura) * static_cast<size_t>(altura));

    // As colunas da grade e a vinheta so dependem do x: calculadas uma vez.
    std::vector<int> colunas(static_cast<size_t>(largura));
    std::vector<float> vinheta(static_cast<size_t>(largura));
    const float meio = static_cast<float>(largura) / 2.0f;
    for (int x = 0; x < largura; ++x) {
        colunas[static_cast<size_t>(x)] = LinhaDaGrade(x, escala);
        const float d = (static_cast<float>(x) + 0.5f - meio) / meio;
        vinheta[static_cast<size_t>(x)] = 1.0f - kVinheta * d * d;
    }

    for (int y = 0; y < altura; ++y) {
        const float t = altura > 1 ? static_cast<float>(y) / static_cast<float>(altura - 1) : 0.0f;
        const float r = Canal(kTopo.r, kBase.r, t);
        const float g = Canal(kTopo.g, kBase.g, t);
        const float b = Canal(kTopo.b, kBase.b, t);
        const int linha = LinhaDaGrade(y, escala);

        std::uint32_t* saida = pixels.data() + static_cast<size_t>(y) * static_cast<size_t>(largura);
        for (int x = 0; x < largura; ++x) {
            const int grade = std::max(linha, colunas[static_cast<size_t>(x)]);
            const float mistura = grade == 2 ? kLinhaForte : (grade == 1 ? kLinhaFraca : 0.0f);
            const float v = vinheta[static_cast<size_t>(x)];
            const auto c = [&](const float base, const std::uint8_t grade_) {
                const float valor = (base + (static_cast<float>(grade_) - base) * mistura) * v;
                return static_cast<std::uint32_t>(std::clamp(std::lround(valor), 0L, 255L));
            };
            saida[x] = 0xFF000000u | (c(r, kCorDaGrade.r) << 16) | (c(g, kCorDaGrade.g) << 8) | c(b, kCorDaGrade.b);
        }
    }
    return pixels;
}

}
