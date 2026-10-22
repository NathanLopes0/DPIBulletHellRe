//
// O ciclo de um agente de IA: pensa, age, observa.
//

#include "ReAct.h"

#include <cmath>

namespace ReAct {

    namespace {
        float Volta(const Ciclo& c) { return c.pensa + c.age + c.observa; }
    }

    Passo PassoDe(const Ciclo& c) {
        if (c.t < c.pensa) return Passo::Pensa;
        if (c.t < c.pensa + c.age) return Passo::Age;
        return Passo::Observa;
    }

    bool Avancar(Ciclo& c, const float deltaTime) {
        const float volta = Volta(c);
        if (volta <= 0.0f || deltaTime <= 0.0f) return false;

        // O AGE COMECA EM c.pensa: agiu quem cruzou esse ponto neste quadro,
        // contando a volta inteira que o quadro possa ter dado.
        const float antes = c.t;
        const float depois = antes + deltaTime;
        const bool cruzou = (antes < c.pensa && depois >= c.pensa)
                         || (depois >= volta + c.pensa);
        c.t = std::fmod(depois, volta);
        return cruzou;
    }
}
