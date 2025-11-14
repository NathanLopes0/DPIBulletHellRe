//
// Created by gensh on 11/11/2025.
//

#include "ProgressBarComponent.h"
#include "../../Actors/Actor.h"
#include "SDL.h"

ProgressBarComponent::ProgressBarComponent(Actor* owner, int drawOrder)
    : DrawComponent(owner, drawOrder)
    , mFillPercent(0.0f)
    , mWidth(200)
    , mHeight(200)
    , mBackgroundColor({50,50,50, 255})
    , mStartColor({255, 100, 150, 255})
    , mOrientation(Orientation::Horizontal)
    , mUseGradient(false)
    , mEffectTimer(0.0f)
    , mEffect(VisualEffect::None)
    , mColorPercent(0.0f)
    , mEndColor({0,0,0,0})
{

}

void ProgressBarComponent::Update(float deltaTime) {
    if (mEffect == VisualEffect::Pulsing) {
        mEffectTimer += deltaTime;
    }
}


void ProgressBarComponent::Draw(SDL_Renderer* renderer) {

    if (!mIsVisible) {
        return;
    }

    Vector2 ownerPos = mOwner->GetPosition();
    const int posX = static_cast<int>(ownerPos.x);
    const int posY = static_cast<int>(ownerPos.y);

    // 1. Desenha o Fundo (sempre igual)
    SDL_Rect bgRect;
    bgRect.x = posX;
    bgRect.y = posY;
    bgRect.w = mWidth;
    bgRect.h = mHeight;
    SDL_SetRenderDrawColor(renderer, mBackgroundColor.r, mBackgroundColor.g, mBackgroundColor.b, mBackgroundColor.a);
    SDL_RenderFillRect(renderer, &bgRect);


    // 2. Desenha o Preenchimento
    if (mUseGradient) {
        // --- LÓGICA DE COR SÓLIDA INTERPOLADA ---

        // 1. Calcular a cor-alvo ÚNICA baseada no *percentual de cor*
        SDL_Color targetColor = LerpColor(mStartColor, mEndColor, mColorPercent);

        // 2. Aplicar efeitos visuais
        if (mEffect == VisualEffect::Pulsing) {
            float pulse = (sinf(mEffectTimer * 8.0f) + 1.0f) / 2.0f;
            float alphaMod = pulse;
            targetColor.a = static_cast<Uint8>(static_cast<float>(targetColor.a) * alphaMod);
        }

        // 3. Definir a cor de desenho para essa cor calculada.
        SDL_SetRenderDrawColor(renderer, targetColor.r, targetColor.g, targetColor.b, targetColor.a);

        // 4. Desenhar um ÚNICO retângulo de preenchimento (baseado no *percentual de preenchimento*)
        SDL_Rect fgRect;
        fgRect.x = posX;
        fgRect.y = posY;

        if (mOrientation == Orientation::Horizontal) {
            if (const int fgWidth = static_cast<int>(static_cast<float>(mWidth) * mFillPercent); fgWidth > 0) {
                fgRect.w = fgWidth;
                fgRect.h = mHeight;
                SDL_RenderFillRect(renderer, &fgRect);
            }
        } else { // Vertical
            if (const int fgHeight = static_cast<int>(static_cast<float>(mHeight) * mFillPercent); fgHeight > 0) {
                const int emptyHeight = mHeight - fgHeight;
                fgRect.w = mWidth;
                fgRect.h = fgHeight;
                fgRect.y = posY + emptyHeight;
                SDL_RenderFillRect(renderer, &fgRect);
            }
        }

    } else {
        // --- COR SÓLIDA ---
        SDL_SetRenderDrawColor(renderer, mStartColor.r, mStartColor.g, mStartColor.b, mStartColor.a);

        SDL_Rect fgRect;
        fgRect.x = posX;
        fgRect.y = posY;

        if (mOrientation == Orientation::Horizontal) {
            if (const int fgWidth = static_cast<int>(static_cast<float>(mWidth) * mFillPercent); fgWidth > 0) {
                fgRect.w = fgWidth;
                fgRect.h = mHeight;
                SDL_RenderFillRect(renderer, &fgRect);
            }
        } else { // Vertical
            if (const int fgHeight = static_cast<int>(static_cast<float>(mHeight) * mFillPercent); fgHeight > 0) {
                const int emptyHeight = mHeight - fgHeight;
                fgRect.w = mWidth;
                fgRect.h = fgHeight;
                fgRect.y = posY + emptyHeight;
                SDL_RenderFillRect(renderer, &fgRect);
            }
        }
    }
}

void ProgressBarComponent::SetFillPercent(const float percent) {
    mFillPercent = Math::Max(0.0f, Math::Min(percent, 1.0f));
}

void ProgressBarComponent::SetColorPercent(const float percent) {
    mColorPercent = Math::Max(0.0f, Math::Min(percent, 1.0f));
}

void ProgressBarComponent::SetColors(SDL_Color background, SDL_Color foreground) {
    mBackgroundColor = background;
    mStartColor = foreground;
    mUseGradient = false;
}

void ProgressBarComponent::SetGradientColors(SDL_Color background, SDL_Color start, SDL_Color end) {
    mBackgroundColor = background;
    mStartColor = start;
    mEndColor = end;
    mUseGradient = true;
}


void ProgressBarComponent::SetSize(const int width, const int height) {
    mWidth = width;
    mHeight = height;
}

SDL_Color ProgressBarComponent::LerpColor(const SDL_Color colorA, const SDL_Color colorB, float alpha) {
    alpha = Math::Clamp(alpha, 0.0f, 1.0f);
    return {
        static_cast<Uint8>(static_cast<float>(colorA.r) + (static_cast<float>(colorB.r) - static_cast<float>(colorA.r)) * alpha),
        static_cast<Uint8>(static_cast<float>(colorA.g) + (static_cast<float>(colorB.g) - static_cast<float>(colorA.g)) * alpha),
        static_cast<Uint8>(static_cast<float>(colorA.b) + (static_cast<float>(colorB.b) - static_cast<float>(colorA.b)) * alpha),
        static_cast<Uint8>(static_cast<float>(colorA.a) + (static_cast<float>(colorB.a) - static_cast<float>(colorA.a)) * alpha)
    };
}
