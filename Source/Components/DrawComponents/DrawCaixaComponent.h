//
// Uma caixa desenhada: moldura, e preenchimento opcional.
//

#pragma once

#include <SDL_pixels.h>

#include "DrawComponent.h"

/**
 * @class DrawCaixaComponent
 * @brief Desenha um retangulo centrado no ator: moldura e, se quiser, fundo.
 *
 * Existe porque nao havia como desenhar uma moldura sem um sprite. Um campo em que
 * o usuario digita precisa de borda - sem ela o texto fica solto no meio da tela e
 * ninguem sabe onde esta escrevendo.
 *
 * Nao e uma barra de progresso nem um botao: so a moldura. Quem quiser texto
 * dentro poe um DrawTextComponent com ordem de desenho MAIOR - a lista e desenhada
 * da menor para a maior, entao a maior fica por cima.
 */
class DrawCaixaComponent : public DrawComponent {

public:
    DrawCaixaComponent(class Actor* owner, int largura, int altura,
                       SDL_Color moldura, SDL_Color fundo,
                       int espessura = 3, int drawOrder = 110);

    void Draw(SDL_Renderer* renderer) override;

    void SetMoldura(const SDL_Color cor) { mMoldura = cor; }

private:
    int mLargura;
    int mAltura;
    SDL_Color mMoldura;
    SDL_Color mFundo;
    int mEspessura;
};
