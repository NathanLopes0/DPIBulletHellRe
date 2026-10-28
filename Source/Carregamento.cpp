//
// A tela de carregamento da batalha: o ritmo da barra e do veu.
//

#include "Carregamento.h"

#include <algorithm>

namespace Carregamento {

    void Avancar(Estado& e, const float feito, const float dt) {
        const float alvo = std::clamp(feito, 0.0f, 1.0f);
        e.mostrado = std::min(alvo, e.mostrado + kRapidezMaxima * dt);
        if (e.mostrado >= 1.0f) e.cheia = e.cheia < 0.0f ? 0.0f : e.cheia + dt;
    }

    bool Pronto(const Estado& e) { return e.cheia >= kSegura; }

    bool Acabou(const Estado& e) { return e.cheia >= kSegura + kSome; }

    float Opacidade(const Estado& e) {
        if (!Pronto(e)) return 1.0f;
        return std::clamp(1.0f - (e.cheia - kSegura) / kSome, 0.0f, 1.0f);
    }
}
