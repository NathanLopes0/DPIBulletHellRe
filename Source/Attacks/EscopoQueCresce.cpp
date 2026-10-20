//
// O escopo que cresce a cada rajada - a regra, sem nada do motor.
//

#include "EscopoQueCresce.h"

namespace Escopo {

Acrescimo DaRajada(const int sprint, const int passoProjeteis, const float passoLeque) {

    const int voltas = (sprint > 0) ? sprint : 0;

    Acrescimo a;
    a.projeteis = voltas * passoProjeteis;
    a.leque     = static_cast<float>(voltas) * passoLeque;
    return a;
}

int SprintSeguinte(const int sprint, const int sprintsAteAEntrega) {

    if (sprintsAteAEntrega < 1) return 0;

    const int seguinte = ((sprint > 0) ? sprint : 0) + 1;
    return (seguinte >= sprintsAteAEntrega) ? 0 : seguinte;
}

}
