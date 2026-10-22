//
// O Blink: a fase final do Ricardo.
//

#include "Blink.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>

namespace Blink {

    std::optional<float> LerDelay(const std::string& placar) {
        const std::size_t abre = placar.find("delay(");
        if (abre == std::string::npos) return std::nullopt;
        const std::size_t inicio = abre + 6;
        const std::size_t fecha = placar.find(')', inicio);
        if (fecha == std::string::npos || fecha == inicio) return std::nullopt;

        // SO DIGITOS: o delay do Arduino e um inteiro de milissegundos.
        const std::string numero = placar.substr(inicio, fecha - inicio);
        if (!std::all_of(numero.begin(), numero.end(), [](const char c) { return std::isdigit(static_cast<unsigned char>(c)) != 0; })) {
            return std::nullopt;
        }
        const long ms = std::strtol(numero.c_str(), nullptr, 10);
        if (ms <= 0) return std::nullopt;
        return static_cast<float>(ms) / 1000.0f;
    }

    bool EsquerdaAcesa(const float t, const float delay) {
        if (!(delay > 0.0f)) return true;
        const auto meios = static_cast<long>(std::floor(std::max(0.0f, t) / delay));
        return meios % 2 == 0;
    }

    bool Trocou(const float antes, const float agora, const float delay) {
        if (!(delay > 0.0f) || agora <= antes) return false;
        return std::floor(agora / delay) > std::floor(std::max(0.0f, antes) / delay);
    }
}
