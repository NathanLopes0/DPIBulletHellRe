//
// Created by gensh on 11/11/2025.
//

#pragma once
#include "DrawComponent.h"
#include "SDL_pixels.h"


class ProgressBarComponent : public DrawComponent {


public:

    enum class Orientation {
        Horizontal,
        Vertical
    };

    enum class VisualEffect {
        None,
        Pulsing
    };

    /**
     * @brief O ACABAMENTO: moldura, sulco e o brilho na ponta do preenchimento.
     *
     * NAO e o padrao, e e de proposito. As duas barras do jogo pedem coisas
     * opostas: a do TEMPO e uma lasca vertical de 10 px colada no chefe, e
     * moldura nela so atrapalharia - ela tem de sumir no cenario. A da NOTA
     * fica sozinha numa faixa larga na base da tela, e sem acabamento le-se
     * como um retangulo cru riscado na tela.
     *
     * As cores do acabamento SAEM DA COR DO PREENCHIMENTO, clareada e
     * escurecida. Nao ha paleta nova aqui: a nota muda de vermelho para laranja
     * e para verde ao longo da batalha, e um conjunto fixo de tons brigaria com
     * duas das tres faixas.
     */
    void SetAcabamento(const bool ligado) { mAcabamento = ligado; }

    explicit ProgressBarComponent(Actor* owner, int drawOrder = 200);

    void Draw(SDL_Renderer *renderer) override;
    void Update(float deltaTime) override;

    void SetOrientation(Orientation orientation) { mOrientation = orientation;}
    void SetFillPercent(float percent);
    void SetColorPercent(float percent);
    void SetColors(SDL_Color background, SDL_Color foreground);
    void SetGradientColors(SDL_Color background, SDL_Color start, SDL_Color end);
    void SetSize(int width, int height);

    void SetEffect(const VisualEffect effect) { mEffect = effect; }

private:
    float mFillPercent;
    float mColorPercent{};

    int mWidth;
    int mHeight;

    SDL_Color mBackgroundColor;
    SDL_Color mStartColor;
    SDL_Color mEndColor{};

    bool mUseGradient;

    VisualEffect mEffect;
    float mEffectTimer{};

    bool mAcabamento = false;

    static SDL_Color LerpColor(SDL_Color colorA, SDL_Color colorB, float alpha);

    /// Um tom da MESMA cor, mais claro ou mais escuro. E o que permite o
    /// acabamento acompanhar o vermelho, o laranja e o verde sem tres paletas.
    static SDL_Color Clarear(SDL_Color cor, float quanto);
    static SDL_Color Escurecer(SDL_Color cor, float quanto);

    /// Desenha a moldura e o sulco por baixo do preenchimento.
    void DesenharSulco(SDL_Renderer* renderer, const SDL_Rect& caixa) const;

    /// Desenha o brilho do topo e a ponta acesa, por cima do preenchimento.
    void DesenharBrilho(SDL_Renderer* renderer, const SDL_Rect& preenchido,
                        SDL_Color cor) const;

    Orientation mOrientation;

};