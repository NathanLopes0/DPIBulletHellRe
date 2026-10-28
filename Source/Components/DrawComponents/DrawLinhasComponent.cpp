//
// Segmentos de reta desenhados: ponteiros, buscas e chamadas no campo.
//

#include "DrawLinhasComponent.h"

#include <cmath>
#include <SDL_render.h>

#include "../../Actors/Actor.h"

DrawLinhasComponent::DrawLinhasComponent(Actor* owner, const int drawOrder)
    : DrawComponent(owner, drawOrder)
{
}

void DrawLinhasComponent::Draw(SDL_Renderer* renderer) {
    if (!IsVisible() || mLinhas.empty()) return;

    // UM DONO PARADO NAO DESENHA. O laco de desenho percorre todos os
    // desenhaveis sem olhar o estado, e no fim da batalha tudo e pausado: sem
    // isto, o grafo do ultimo quadro ficaria boiando por cima da tela de nota.
    if (mOwner->GetState() != ActorState::Active) return;

    SDL_BlendMode anterior;
    SDL_GetRenderDrawBlendMode(renderer, &anterior);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    for (const Linha& l : mLinhas) {
        SDL_SetRenderDrawColor(renderer, l.cor.r, l.cor.g, l.cor.b, l.cor.a);

        // A SEGUNDA LINHA VAI DE LADO, um pixel para fora no eixo menos
        // inclinado: deslocar sempre no y deixaria a linha vertical com 1 px.
        const float dx = l.ate.x - l.de.x, dy = l.ate.y - l.de.y;
        const int ox = (std::fabs(dy) > std::fabs(dx)) ? 1 : 0;
        const int oy = 1 - ox;
        const int x1 = static_cast<int>(l.de.x), y1 = static_cast<int>(l.de.y);
        const int x2 = static_cast<int>(l.ate.x), y2 = static_cast<int>(l.ate.y);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        SDL_RenderDrawLine(renderer, x1 + ox, y1 + oy, x2 + ox, y2 + oy);
    }

    SDL_SetRenderDrawBlendMode(renderer, anterior);
}
