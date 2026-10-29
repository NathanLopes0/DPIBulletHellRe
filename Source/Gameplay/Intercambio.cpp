//
// O Programa de Intercambio do Vladimir - para onde cada aluno vai, e quando.
//

#include "Intercambio.h"

namespace Intercambio {

std::vector<Vector2> Destinos(const std::vector<Vector2>& posicoes, const float largura) {

    std::vector<Vector2> destinos;
    if (posicoes.empty()) return destinos;

    if (posicoes.size() == 1) {
        destinos.emplace_back(largura - posicoes[0].x, posicoes[0].y);
        return destinos;
    }

    destinos.reserve(posicoes.size());
    for (size_t i = 0; i < posicoes.size(); ++i) {
        destinos.push_back(posicoes[(i + 1) % posicoes.size()]);
    }
    return destinos;
}

bool Valido(const Relogio& r) {
    return r.aCada > 0.0f && r.aviso >= 0.0f && r.aviso < r.aCada;
}

bool Avancar(Relogio& r, const float deltaTime) {
    if (!Valido(r)) return false;

    r.t += deltaTime;
    if (r.t < r.aCada) return false;

    r.t -= r.aCada;
    return true;
}

bool Avisando(const Relogio& r) {
    return Valido(r) && r.t >= r.aCada - r.aviso;
}

}
