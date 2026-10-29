//
// A janela ancorada: o modal que acompanha a janela-mae.
//

#pragma once

#include "../Core/Math.h"

/**
 * CAMADA PURA.
 *
 * Na fase final da Maria Lucia as janelas MODAIS ficam presas a professora,
 * como um dialogo fica centrado na janela que o abriu: cada uma tem um lugar
 * em relacao a ela, vai para la e acompanha - ate o tempo acabar, e entao cai.
 *
 * Quem faz isso no jogo e o AncorarBehavior (Behaviors.h); aqui fica o passo
 * da aproximacao, coberto por test_ancora.cpp.
 */
namespace Ancora {

    /// Quao depressa a janela alcanca o lugar dela: a cada segundo, sobra
    /// e^-taxa do que faltava.
    constexpr float kTaxa = 8.0f;

    /// Um passo da aproximacao: anda a fracao 1 - e^(-taxa * dt) do que falta.
    /// Nao depende do quadro: dois passos de meio quadro chegam onde um passo
    /// de um quadro chega.
    Vector2 Aproximar(Vector2 atual, Vector2 alvo, float taxa, float deltaTime);
}
