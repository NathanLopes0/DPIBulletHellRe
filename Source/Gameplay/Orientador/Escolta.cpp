//
// O orientador em campo: onde ele fica, e para onde atira.
//

#include "Escolta.h"

#include <algorithm>
#include <cmath>

namespace Escolta {

    int Lado(const float xDoAluno, const int ladoAtual, const float esquerda, const float direita) {
        const int lado = ladoAtual < 0 ? -1 : 1;
        const float x = xDoAluno + static_cast<float>(lado) * kAoLado;
        if (lado < 0 && x < esquerda + kMargem) return 1;
        if (lado > 0 && x > direita - kMargem) return -1;
        return lado;
    }

    Vector2 Posto(const std::vector<Vector2>& alunos, const int lado, const float esquerda, const float direita) {
        const float minimo = esquerda + kMargem;
        const float maximo = std::max(minimo, direita - kMargem);

        if (alunos.empty()) return Vector2((esquerda + direita) / 2.0f, kAcimaDaDupla);

        Vector2 posto;
        if (alunos.size() == 1) {
            posto = alunos.front() + Vector2(static_cast<float>(lado < 0 ? -1 : 1) * kAoLado, -kAcima);
        } else {
            Vector2 soma = Vector2::Zero;
            for (const auto& a : alunos) soma += a;
            posto = soma * (1.0f / static_cast<float>(alunos.size())) + Vector2(0.0f, -kAcimaDaDupla);
        }
        posto.x = std::clamp(posto.x, minimo, maximo);
        return posto;
    }

    Vector2 Seguir(const Vector2& atual, const Vector2& alvo, const float dt) {
        if (dt <= 0.0f) return atual;
        const float fracao = 1.0f - std::exp(-kRapidez * dt);
        return atual + (alvo - atual) * fracao;
    }
}
