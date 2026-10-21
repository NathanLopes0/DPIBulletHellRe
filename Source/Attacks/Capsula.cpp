//
// A capsula: in capsula, "na caixinha".
//

#include "Capsula.h"

#include <cmath>

namespace Capsula {

std::vector<Vector2> Abertura(const int n, const float anguloDoVoo, const float velocidade) {
    std::vector<Vector2> v;
    if (n <= 0) return v;
    v.reserve(static_cast<size_t>(n));
    constexpr float kGrau = 3.14159265f / 180.0f;
    for (int i = 0; i < n; ++i) {
        const float a = (anguloDoVoo + 360.0f * static_cast<float>(i) / static_cast<float>(n)) * kGrau;
        v.emplace_back(std::cos(a) * velocidade, std::sin(a) * velocidade);
    }
    return v;
}

}
