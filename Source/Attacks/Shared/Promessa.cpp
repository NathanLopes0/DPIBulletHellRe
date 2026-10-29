//
// A Promise do JavaScript - a decisao de resolver ou rejeitar.
//

#include "Promessa.h"

namespace Promessa {

bool Resolve(const float sorteio, const float fracao) {
    if (fracao >= 1.0f) return true;
    return sorteio < fracao;
}

}
