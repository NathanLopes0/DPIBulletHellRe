//
// A amostragem com temperatura de um modelo de linguagem.
//

#include "Temperatura.h"

#include <algorithm>
#include <cmath>

namespace Temperatura {

    std::vector<float> Probabilidades(const std::vector<float>& logits, const float temperatura) {
        std::vector<float> p(logits.size(), 0.0f);
        if (logits.empty()) return p;

        const auto maior = std::max_element(logits.begin(), logits.end());
        if (temperatura <= 0.0f) {
            p[static_cast<size_t>(maior - logits.begin())] = 1.0f;
            return p;
        }

        // O maior logit sai na frente da exponencial: e^(l - max) nunca estoura.
        float soma = 0.0f;
        for (size_t i = 0; i < logits.size(); ++i) {
            p[i] = std::exp((logits[i] - *maior) / temperatura);
            soma += p[i];
        }
        for (float& x : p) x /= soma;
        return p;
    }

    int Amostrar(const std::vector<float>& probabilidades, const float u) {
        float acumulada = 0.0f;
        int ultimo = -1;
        for (size_t i = 0; i < probabilidades.size(); ++i) {
            if (probabilidades[i] <= 0.0f) continue;
            ultimo = static_cast<int>(i);
            acumulada += probabilidades[i];
            if (u < acumulada) return ultimo;
        }
        return ultimo < 0 ? 0 : ultimo;
    }

    Candidatos DeAngulo(const int n, const float abertura, const float escala) {
        Candidatos c;
        if (n <= 0) return c;
        for (int k = 0; k < n; ++k) {
            const float desvio = (n == 1) ? 0.0f
                               : -0.5f * abertura + abertura * static_cast<float>(k) / static_cast<float>(n - 1);
            c.desvios.push_back(desvio);
            c.logits.push_back(-std::fabs(desvio) / (escala > 0.0f ? escala : 1.0f));
        }
        return c;
    }
}
