//
// A geracao de texto da fase 1 do Julio.
//

#include "Geracao.h"

#include <cmath>

namespace Geracao {

    float Diferenca(const float de, const float para) {
        float d = std::fmod(para - de, 360.0f);
        if (d > 180.0f) d -= 360.0f;
        if (d <= -180.0f) d += 360.0f;
        return d;
    }

    float RumoProvavel(const float rumo, const float aoAlvo, const float virada) {
        const float d = Diferenca(rumo, aoAlvo);
        const float v = std::fabs(virada);
        return rumo + (d > v ? v : (d < -v ? -v : d));
    }

    Passo Proximo(const float rumo, const float aoAlvo, const float temperatura, const float u) {
        Passo p;
        const float centro = RumoProvavel(rumo, aoAlvo);
        const Temperatura::Candidatos c = Temperatura::DeAngulo(kCandidatos, kAbertura);
        for (const float desvio : c.desvios) p.rumos.push_back(centro + desvio);
        p.probabilidades = Temperatura::Probabilidades(c.logits, temperatura);
        p.escolhido = Temperatura::Amostrar(p.probabilidades, u);
        return p;
    }

    int TokensDaFrase(const float temperatura) {
        if (temperatura <= 1.0f) return kTokensDaFrase;
        return kTokensDaFrase + static_cast<int>(std::lround((temperatura - 1.0f) / 0.2f));
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.entreFrases = Dificuldade::Escolher(n, d.entreFrases, 1.75f);
        return d;
    }
}
