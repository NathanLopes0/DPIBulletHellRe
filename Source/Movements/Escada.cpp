//
// A escada do modelo cascata.
//

#include "Escada.h"

#include <algorithm>
#include <cmath>

namespace Escada {

Vector2 Degrau(int k, const int n, const Geometria& g) {
    if (n <= 1) return Vector2(g.margem, g.topo);
    k = std::max(0, std::min(n - 1, k));
    const float largura = g.largura - 2.0f * g.margem;
    const float x = g.margem + largura * static_cast<float>(k) / static_cast<float>(n - 1);
    return Vector2(x, g.topo + g.passo * static_cast<float>(k));
}

int DegrauNoTempo(const float t, const float segundosPorDegrau, const int n) {
    if (n <= 0 || segundosPorDegrau <= 0.0f || t <= 0.0f) return 0;
    const int k = static_cast<int>(std::floor(t / segundosPorDegrau));
    return std::min(k, n - 1);
}

}
