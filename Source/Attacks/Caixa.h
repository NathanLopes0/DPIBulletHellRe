//
// A caixa do HTML - a geometria de um retangulo que cresce sem perder a forma.
//

#pragma once

#include <vector>

#include "../Motor/Math.h"

/// A fase 1 do Lucas e o HTML: caixas que nascem em volta dele e crescem, uma dentro da outra, como
/// divs aninhadas.
namespace Caixa {

    /// 'n' pontos igualmente espacados pelo contorno de um retangulo centrado na origem, em sentido
    /// horario a partir do canto de cima a esquerda.
    std::vector<Vector2> PontosDoContorno(int n, float meiaLargura, float meiaAltura);

    /// A velocidade de um ponto do contorno para a caixa crescer sem deformar.
    Vector2 Velocidade(const Vector2& ponto, float meiaAlturaInicial, float velocidade);
}
