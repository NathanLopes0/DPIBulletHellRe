//
// A capivara que atravessa o campo do Salles.
//

#include "Capivara.h"

namespace Capivara {

    Visita Sortear(const std::uint32_t sorteio) {
        // Tres partes do sorteio, uma para cada pergunta.
        const float vem = static_cast<float>(sorteio & 1023u) / 1024.0f;
        const float quando = static_cast<float>((sorteio >> 10) & 1023u) / 1023.0f;
        Visita v;
        v.vem = vem < kChance;
        v.quando = kDe + (kAte - kDe) * quando;
        v.daEsquerda = ((sorteio >> 20) & 1u) == 0u;
        return v;
    }
}
