//
// Em quem o professor mira, na dupla: em quem mais o incomodou nos ultimos segundos.
//

#include "Papeis.h"

#include <cmath>

namespace Papeis {

    void Incomodo::Acertou(const std::size_t quem, const float peso) {
        if (quem < kAlunos && peso > 0.0f) mValor[quem] += peso;
    }

    void Incomodo::Avancar(const float deltaTime) {
        if (deltaTime <= 0.0f) return;
        const float resta = std::exp2(-deltaTime / kMeiaVida);
        for (float& v : mValor) v *= resta;
    }

    float Incomodo::De(const std::size_t quem) const {
        return quem < kAlunos ? mValor[quem] : 0.0f;
    }

    int Incomodo::Foco(const std::size_t quantos) const {
        if (quantos < 2) return -1;
        const float a = mValor[0], b = mValor[1];
        if (a >= kRazao * b + kMinimo) return 0;
        if (b >= kRazao * a + kMinimo) return 1;
        return -1;
    }

    std::size_t Incomodo::Alvo(const std::size_t vez, const std::size_t quantos) const {
        if (quantos < 2) return 0;
        const int foco = Foco(quantos);
        return foco >= 0 ? static_cast<std::size_t>(foco) : vez % quantos;
    }
}
