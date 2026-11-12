//
// Uma caixa desenhada: moldura, e preenchimento opcional.
//

#pragma once

#include <SDL_pixels.h>

#include "DrawComponent.h"

/// Desenha um retangulo centrado no ator: moldura e, se quiser, fundo. Existe porque nao havia como
/// desenhar uma moldura sem um sprite.
class DrawCaixaComponent : public DrawComponent {

public:
    DrawCaixaComponent(class Actor* owner, int largura, int altura,
                       SDL_Color moldura, SDL_Color fundo,
                       int espessura = 3, int drawOrder = 110);

    void Draw(SDL_Renderer* renderer) override;

    void SetMoldura(const SDL_Color cor) { mMoldura = cor; }
    void SetFundo(const SDL_Color cor) { mFundo = cor; }

private:
    int mLargura;
    int mAltura;
    SDL_Color mMoldura;
    SDL_Color mFundo;
    int mEspessura;
};
