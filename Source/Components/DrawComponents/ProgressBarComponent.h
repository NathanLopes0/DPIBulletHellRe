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

    static SDL_Color LerpColor(SDL_Color colorA, SDL_Color colorB, float alpha);

    Orientation mOrientation;

};