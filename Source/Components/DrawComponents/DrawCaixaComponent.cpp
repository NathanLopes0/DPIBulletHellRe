//
// Uma caixa desenhada: moldura, e preenchimento opcional.
//

#include "DrawCaixaComponent.h"

#include <SDL_render.h>

#include "../../Actors/Actor.h"

DrawCaixaComponent::DrawCaixaComponent(Actor* owner, const int largura, const int altura,
                                       const SDL_Color moldura, const SDL_Color fundo,
                                       const int espessura, const int drawOrder)
    : DrawComponent(owner, drawOrder),
      mLargura(largura), mAltura(altura),
      mMoldura(moldura), mFundo(fundo), mEspessura(espessura)
{
}

void DrawCaixaComponent::Draw(SDL_Renderer* renderer) {

    if (!IsVisible()) return;

    const Vector2 pos = mOwner->GetPosition();
    const SDL_Rect fora = {static_cast<int>(pos.x) - mLargura / 2,
                           static_cast<int>(pos.y) - mAltura / 2,
                           mLargura, mAltura};

    // A mistura precisa estar ligada para fundo com alfa funcionar; sem isso um
    // fundo semitransparente sai solido.
    SDL_BlendMode anterior;
    SDL_GetRenderDrawBlendMode(renderer, &anterior);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    if (mFundo.a > 0) {
        SDL_SetRenderDrawColor(renderer, mFundo.r, mFundo.g, mFundo.b, mFundo.a);
        SDL_RenderFillRect(renderer, &fora);
    }

    // A moldura sao quatro retangulos cheios, e nao SDL_RenderDrawRect, porque
    // aquele desenha sempre com um pixel de espessura.
    SDL_SetRenderDrawColor(renderer, mMoldura.r, mMoldura.g, mMoldura.b, mMoldura.a);
    const SDL_Rect lados[4] = {
        {fora.x, fora.y, fora.w, mEspessura},                          // topo
        {fora.x, fora.y + fora.h - mEspessura, fora.w, mEspessura},     // base
        {fora.x, fora.y, mEspessura, fora.h},                           // esquerda
        {fora.x + fora.w - mEspessura, fora.y, mEspessura, fora.h}      // direita
    };
    SDL_RenderFillRects(renderer, lados, 4);

    SDL_SetRenderDrawBlendMode(renderer, anterior);
}
