//
// A computacao em nuvem do Vitor: processa, e chove pacote.
//

#include "Nuvem.h"

#include <cmath>

namespace Nuvem {

    bool Processando(const Ciclo& c) {
        return c.t < c.processa;
    }

    bool Avancar(Ciclo& c, const float deltaTime) {
        const float volta = c.processa + c.descansa;
        if (volta <= 0.0f || deltaTime <= 0.0f) return false;

        // CHOVE QUANDO O PROCESSAMENTO ACABA, em c.processa: chove quem cruzou
        // esse ponto neste quadro, contando a volta inteira que o quadro possa
        // ter dado.
        const float antes = c.t;
        const float depois = antes + deltaTime;
        const bool chove = (antes < c.processa && depois >= c.processa)
                        || (depois >= volta + c.processa);
        c.t = std::fmod(depois, volta);
        return chove;
    }
}
