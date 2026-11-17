//
// O modo de plataforma do aluno.
//

#include "Plataforma.h"

#include <algorithm>

namespace Plataforma {

    Vector2 Direcao(const Lado l) {
        switch (l) {
            case Lado::Baixo:    return Vector2(0.0f, 1.0f);
            case Lado::Esquerda: return Vector2(-1.0f, 0.0f);
            case Lado::Cima:     return Vector2(0.0f, -1.0f);
            case Lado::Direita:  return Vector2(1.0f, 0.0f);
        }
        return Vector2(0.0f, 1.0f);
    }

    Vector2 AoLongo(const Lado l) {
        const Vector2 g = Direcao(l);
        return Vector2(g.y, -g.x);
    }

    float Giro(const Lado l) {
        switch (l) {
            case Lado::Baixo:    return 0.0f;
            case Lado::Esquerda: return 90.0f;
            case Lado::Cima:     return 180.0f;
            case Lado::Direita:  return 270.0f;
        }
        return 0.0f;
    }

    void Virar(Estado& e) {
        e.queda = kBatida;
        e.noChao = false;
    }

    Vector2 Passo(Estado& e, const Lado l, const Vector2 intencao, const bool encostou, const float dt,
                  const float andar) {
        const Vector2 g = Direcao(l);
        const Vector2 t = AoLongo(l);
        const float vt = std::clamp(Vector2::Dot(intencao, t), -andar, andar);
        const bool pula = Vector2::Dot(intencao, g) < -0.5f * andar;

        // No chao a queda para: o lado do campo segura o aluno. Quem acabou de pular ainda encosta
        // neste quadro, mas ja esta subindo.
        e.noChao = encostou && e.queda >= 0.0f;
        if (e.noChao) e.queda = 0.0f;
        if (e.noChao && pula) {
            e.queda = -kPulo;
            e.noChao = false;
        }
        const bool subindo = e.queda < 0.0f;
        if (subindo && !pula) e.queda = std::max(e.queda, -kCorteDoPulo);
        const float puxa = subindo && pula ? kGravidadeSegurando : kGravidade;
        e.queda = std::min(e.queda + puxa * dt, kQuedaMaxima);
        return t * vt + g * e.queda;
    }

    float AlturaSegurando() {
        return kPulo * kPulo / (2.0f * kGravidadeSegurando);
    }

    float AlturaDoToque() {
        return kCorteDoPulo * kCorteDoPulo / (2.0f * kGravidade);
    }
}
