//
// A fita da maquina de Turing do Vladimir - a regra de quando uma casa e
// reescrita, sem nada do motor.
//

#include "Fita.h"

#include <cmath>

namespace Fita {

bool CabecaSobreACasa(const float xDaCabeca, const float xDaCasa, const float raio) {
    return std::fabs(xDaCabeca - xDaCasa) <= raio;
}

bool DeveReescrever(const float idade, const float idadeMinima,
                    const std::vector<float>& xDasCabecas, const float xDaCasa, const float raio) {

    if (idade < idadeMinima) return false;

    for (const float x : xDasCabecas) {
        if (CabecaSobreACasa(x, xDaCasa, raio)) return true;
    }
    return false;
}

}
