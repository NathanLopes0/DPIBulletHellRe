//
// Quantos pixels da tela cabem num pixel do jogo.
//

#pragma once

#include <algorithm>
#include <SDL_render.h>

/// O fator com que o SDL amplia a tela logica (1200x800) na janela ou na tela cheia. Quem desenha na
/// resolucao real (o texto, o fundo do campo) usa este numero, e redesenha quando ele muda.
inline float EscalaDaTela(SDL_Renderer* renderer) {
    float sx = 1.0f, sy = 1.0f;
    if (renderer) SDL_RenderGetScale(renderer, &sx, &sy);
    // Fora disso e uma janela que nao existe na pratica: a textura nao cresce sem limite.
    return std::clamp(std::min(sx, sy), 0.5f, 4.0f);
}
