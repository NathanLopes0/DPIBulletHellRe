//
// Os guardrails da fase 3 do Julio: cada onda e um retreino.
//

#include "../../doctest.h"

#include "../../../Source/Attacks/Julio/Guardrail.h"

using namespace Guardrail;

TEST_CASE("Guardrail: cada retreino alterna o anel e o giro, e gira mais depressa ate o teto") {
    const Onda primeira = DaVez(0);
    CHECK(primeira.comprimento == kFechado);
    CHECK(primeira.horario);
    CHECK(primeira.rapidez == doctest::Approx(1.0f));
    for (int k = 1; k < 12; ++k) {
        const Onda antes = DaVez(k - 1);
        const Onda agora = DaVez(k);
        CHECK(agora.comprimento != antes.comprimento);
        CHECK(agora.horario != antes.horario);
        CHECK(agora.rapidez >= antes.rapidez);
        CHECK(agora.rapidez <= kRapidezMaxima);
    }
    CHECK(DaVez(11).rapidez == doctest::Approx(kRapidezMaxima));
}
