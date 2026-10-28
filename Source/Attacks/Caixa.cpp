//
// A caixa do HTML - a geometria de um retangulo que cresce sem perder a forma.
//

#include "Caixa.h"

namespace Caixa {

std::vector<Vector2> PontosDoContorno(const int n, const float meiaLargura, const float meiaAltura) {

    std::vector<Vector2> pontos;
    if (n <= 0 || meiaLargura <= 0.0f || meiaAltura <= 0.0f) return pontos;

    const float largura = 2.0f * meiaLargura;
    const float altura  = 2.0f * meiaAltura;
    const float contorno = 2.0f * (largura + altura);
    const float passo = contorno / static_cast<float>(n);

    pontos.reserve(static_cast<size_t>(n));
    for (int i = 0; i < n; ++i) {

        // Quanto ja se andou pelo contorno, a partir do canto de cima a
        // esquerda e em sentido horario: topo, lado direito, base, lado esquerdo.
        float s = passo * static_cast<float>(i);

        if (s < largura) {
            pontos.emplace_back(-meiaLargura + s, -meiaAltura);
            continue;
        }
        s -= largura;
        if (s < altura) {
            pontos.emplace_back(meiaLargura, -meiaAltura + s);
            continue;
        }
        s -= altura;
        if (s < largura) {
            pontos.emplace_back(meiaLargura - s, meiaAltura);
            continue;
        }
        s -= largura;
        pontos.emplace_back(-meiaLargura, meiaAltura - s);
    }
    return pontos;
}

Vector2 Velocidade(const Vector2& ponto, const float meiaAlturaInicial, const float velocidade) {
    if (meiaAlturaInicial <= 0.0f) return Vector2::Zero;
    return ponto * (velocidade / meiaAlturaInicial);
}

}
