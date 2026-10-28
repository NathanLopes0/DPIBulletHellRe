//
// A janela ancorada: o modal que acompanha a janela-mae.
//

#include "Ancora.h"

#include <cmath>

namespace Ancora {

    Vector2 Aproximar(const Vector2 atual, const Vector2 alvo, const float taxa, const float deltaTime) {
        if (taxa <= 0.0f || deltaTime <= 0.0f) return atual;
        const float fracao = 1.0f - std::exp(-taxa * deltaTime);
        return atual + (alvo - atual) * fracao;
    }
}
