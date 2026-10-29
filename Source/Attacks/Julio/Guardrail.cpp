//
// Os guardrails da fase 3 do Julio: cada onda e um retreino.
//

#include "Guardrail.h"

#include <algorithm>

namespace Guardrail {

    Onda DaVez(const int k) {
        Onda o;
        const int n = std::max(0, k);
        o.comprimento = (n % 2 == 0) ? kFechado : kAberto;
        o.horario = (n % 2 == 0);
        o.rapidez = std::min(kRapidezMaxima, 1.0f + kPorRetreino * static_cast<float>(n));
        return o;
    }
}
