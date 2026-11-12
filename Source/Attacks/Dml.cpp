//
// A fase 2 do Thiago.
//

#include "Dml.h"

#include <algorithm>
#include <cmath>

namespace Dml {

Vector2 NoArco(const Vector2& de, const Vector2& para, const float altura, const float t) {
    const float u = std::clamp(t, 0.0f, 1.0f);
    const Vector2 reta = de + (para - de) * u;
    return Vector2(reta.x, reta.y - 4.0f * altura * u * (1.0f - u));
}

float Subida(const float velocidade, const float gravidade) {
    if (gravidade <= 0.0f) return 0.0f;
    return velocidade * velocidade / (2.0f * gravidade);
}

Degrau DegrauDo(const Dificuldade::Nivel n) {
    Degrau d;
    if (n != Dificuldade::Nivel::Dificil) return d;
    d.avisoDoDelete = 0.75f;
    d.avisoDoUpdate = 0.75f;
    d.duracaoDoArco = 0.8f;
    d.gravidade = 1100.0f;
    d.velocidadeDoChute = 430.0f;
    return d;
}

}
