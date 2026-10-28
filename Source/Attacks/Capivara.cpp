//
// A capivara que atravessa o campo do Salles.
//

#include "Capivara.h"

namespace Capivara {

    Visita Sortear(const std::uint32_t sorteio) {
        // TRES PARTES DO SORTEIO, uma para cada pergunta: os 10 bits de baixo
        // decidem se ela vem, os 10 seguintes quando, e o bit 20 o lado.
        const float vem = static_cast<float>(sorteio & 1023u) / 1024.0f;
        const float quando = static_cast<float>((sorteio >> 10) & 1023u) / 1023.0f;
        Visita v;
        v.vem = vem < kChance;
        v.quando = kDe + (kAte - kDe) * quando;
        v.daEsquerda = ((sorteio >> 20) & 1u) == 0u;
        return v;
    }
}
