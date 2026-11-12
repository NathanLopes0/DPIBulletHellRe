//
// Created by gensh on 11/11/2025.
//

#pragma once
#include "DrawComponent.h"
#include "SDL_pixels.h"

/// Uma barra horizontal que enche, com a cor andando de um tom a outro conforme enche, e com
/// acabamento: moldura, sulco e o brilho na ponta.
class ProgressBarComponent : public DrawComponent {
public:
    explicit ProgressBarComponent(Actor* owner, int drawOrder = 200);

    void Draw(SDL_Renderer *renderer) override;

    void SetFillPercent(float percent);
    void SetColorPercent(float percent);

    /// O fundo (o sulco vazio) e as cores de comeco e de fim do preenchimento.
    void SetGradientColors(SDL_Color background, SDL_Color start, SDL_Color end);
    void SetSize(int width, int height);

private:
    float mFillPercent = 0.0f;
    float mColorPercent = 0.0f;

    int mWidth = 200;
    int mHeight = 200;

    SDL_Color mBackgroundColor{50, 50, 50, 255};
    SDL_Color mStartColor{255, 100, 150, 255};
    SDL_Color mEndColor{0, 0, 0, 0};

    static SDL_Color LerpColor(SDL_Color colorA, SDL_Color colorB, float alpha);

    /// Um tom da mesma cor, mais claro ou mais escuro. E o que permite o acabamento acompanhar a
    /// cor que anda sem uma paleta para cada tom.
    static SDL_Color Clarear(SDL_Color cor, float quanto);
    static SDL_Color Escurecer(SDL_Color cor, float quanto);

    /// Desenha a moldura e o sulco por baixo do preenchimento.
    void DesenharSulco(SDL_Renderer* renderer, const SDL_Rect& caixa) const;

    /// Desenha o brilho do topo e a ponta acesa, por cima do preenchimento.
    void DesenharBrilho(SDL_Renderer* renderer, const SDL_Rect& preenchido,
                        SDL_Color cor) const;
};
