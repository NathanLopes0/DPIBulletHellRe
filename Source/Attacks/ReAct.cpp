//
// O ciclo de um agente de IA: pensa, age, observa.
//

#include "ReAct.h"

#include <algorithm>
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

    float PensaDaVolta(Variacao::Sorteio& s, const float pensa) {
        return s.Real(0.7f * pensa, 1.3f * pensa);
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.pensa = Dificuldade::Escolher(n, kPensa, 0.8f);
        d.observa = Dificuldade::Escolher(n, kObserva, 0.5f);
        return d;
    }

    std::vector<Ponto> Destinos(Variacao::Sorteio& s, const float xDoAluno, const int n, const float largura) {
        std::vector<Ponto> lugares;
        const float esquerda = kMargem;
        const float direita = largura - kMargem;
        const float de = std::max(esquerda, std::min(xDoAluno - kAlcance, direita - 2.0f * kAlcance));
        const float ate = std::min(direita, std::max(xDoAluno + kAlcance, esquerda + 2.0f * kAlcance));
        const auto distancia = [&](const Ponto& p) {
            float menor = 1e9f;
            for (const Ponto& q : lugares) menor = std::min(menor, std::hypot(p.x - q.x, p.y - q.y));
            return menor;
        };
        for (int k = 0; k < n; ++k) {
            Ponto melhor;
            float maisLonge = -1.0f;
            for (int tentativa = 0; tentativa < 24; ++tentativa) {
                const Ponto p{s.Real(de, ate), s.Real(kYMinimo, kYMaximo)};
                const float d = distancia(p);
                if (d > maisLonge) { maisLonge = d; melhor = p; }
                if (d >= kSeparacao) break;
            }
            lugares.push_back(melhor);
        }
        return lugares;
    }
}

