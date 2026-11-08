//
// A janela ancorada: o modal que acompanha a janela-mae.
//

#pragma once

#include "../Motor/Math.h"

/// Na fase final da Maria Lucia as janelas modais ficam presas a professora, como um dialogo fica
/// centrado na janela que o abriu.
namespace Ancora {

    /// Quao depressa a janela alcanca o lugar dela: a cada segundo, sobra
    /// e^-taxa do que faltava.
    constexpr float kTaxa = 8.0f;

    /// Um passo da aproximacao: anda a fracao 1 - e^(-taxa * dt) do que falta.
    Vector2 Aproximar(Vector2 atual, Vector2 alvo, float taxa, float deltaTime);
}
