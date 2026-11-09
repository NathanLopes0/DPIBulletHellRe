//
// A cobertura, na dupla: derrubar o projetil que ia no parceiro.
//

#include "Cobertura.h"

#include <algorithm>

namespace Cobertura {

    bool IaPassarPerto(const Vector2& pos, const Vector2& vel, const Vector2& alvo, const float raio,
                       const float horizonte) {
        const float v2 = vel.LengthSq();
        // O instante do caminho mais perto do alvo, dentro do horizonte; parado, e agora.
        const float t = v2 > 0.0f ? std::clamp(Vector2::Dot(alvo - pos, vel) / v2, 0.0f, horizonte) : 0.0f;
        const Vector2 maisPerto = pos + vel * t;
        return (alvo - maisPerto).LengthSq() <= raio * raio;
    }
}
