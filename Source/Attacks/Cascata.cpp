//
// O modelo cascata da fase 1 da Maria Lucia.
//

#include "Cascata.h"

#include <algorithm>
#include <random>

namespace Cascata {

    int EtapaNoTempo(const float t) {
        if (t < 0.0f) return 0;
        const int k = static_cast<int>(t / kDuracaoDaEtapa);
        return std::min(k, static_cast<int>(kEtapas.size()));
    }

    Vector2 LugarNoBacklog(const int k) {
        const int i = std::clamp(k, 0, kMaxPedidos - 1);
        return Vector2(70.0f + 46.0f * static_cast<float>(i % 5), 222.0f + 44.0f * static_cast<float>(i / 5));
    }

    Vector2 OrigemDoPedido(const std::uint32_t sorteio, const float largura, const float altura) {
        std::mt19937 g(sorteio);
        const bool esquerda = (g() % 2u) == 0u;
        const float de = altura * 0.45f;
        const float ate = altura - 60.0f;
        const float y = de + static_cast<float>(g() % 1000u) / 1000.0f * (ate - de);
        return Vector2(esquerda ? -20.0f : largura + 20.0f, y);
    }

    Vector2 LugarDaClasse(const int k, const float largura) {
        const int i = std::clamp(k, 0, kMaxClasses - 1);
        const float coluna = static_cast<float>(i % kClassesPorLinha);
        const float linha = static_cast<float>(i / kClassesPorLinha);
        return Vector2(largura * (0.15f + 0.2f * coluna), kYDasClasses + kEntreLinhasDeClasses * linha);
    }

    std::vector<std::pair<int, int>> Associacoes(const int classes) {
        std::vector<std::pair<int, int>> setas;
        const int n = std::clamp(classes, 0, kMaxClasses);
        for (int k = 0; k < n; ++k) {
            if (k % kClassesPorLinha != kClassesPorLinha - 1 && k + 1 < n) setas.emplace_back(k, k + 1);
            if (k + kClassesPorLinha < n) setas.emplace_back(k, k + kClassesPorLinha);
        }
        return setas;
    }
}
