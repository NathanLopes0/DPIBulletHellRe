//
// Created by gensh on 11/11/2025.
//

#include "ProgressBarComponent.h"
#include "../../Actors/Actor.h"
#include "SDL.h"

ProgressBarComponent::ProgressBarComponent(Actor* owner, int drawOrder)
    : DrawComponent(owner, drawOrder)
    , mProgress(0.0f)
    , mWidth(200)
    , mHeight(200)
    , mBackgroundColor({50,50,50, 255})
    , mForegroundColor({255, 100, 150, 255})
    , mOrientation(Orientation::Horizontal)
{

}

void ProgressBarComponent::Draw(SDL_Renderer* renderer) {

    if (!mIsVisible) {
        return;
    }

    Vector2 ownerPos = mOwner->GetPosition();

    // 1. Desenha o Fundo (comum para ambos)
    SDL_Rect bgRect;
    bgRect.x = static_cast<int>(ownerPos.x);
    bgRect.y = static_cast<int>(ownerPos.y);
    bgRect.w = mWidth;
    bgRect.h = mHeight;
    SDL_SetRenderDrawColor(renderer, mBackgroundColor.r, mBackgroundColor.g, mBackgroundColor.b, mBackgroundColor.a);
    SDL_RenderFillRect(renderer, &bgRect);

    // --- MUDANÇA NA LÓGICA DE DESENHO ---

    SDL_Rect fgRect; // Retângulo do preenchimento
    fgRect.x = static_cast<int>(ownerPos.x);
    fgRect.y = static_cast<int>(ownerPos.y);

    if (mOrientation == Orientation::Horizontal) { // Lógica pra HORIZONTAL
        if (const int fgWidth = static_cast<int>(static_cast<float>(mWidth) * mProgress); fgWidth > 0) {
            fgRect.w = fgWidth;
            fgRect.h = mHeight;
            SDL_SetRenderDrawColor(renderer, mForegroundColor.r, mForegroundColor.g, mForegroundColor.b, mForegroundColor.a);
            SDL_RenderFillRect(renderer, &fgRect);
        }
    } else { // Lógica pra VERTICAL
        // Calcula a altura baseada no progresso
        if (const int fgHeight = static_cast<int>(static_cast<float>(mHeight) * mProgress); fgHeight > 0) {

            const int emptyHeight = mHeight - fgHeight;
            fgRect.w = mWidth;
            fgRect.h = fgHeight;
            fgRect.x = static_cast<int>(ownerPos.x);

            fgRect.y = static_cast<int>(ownerPos.y) + emptyHeight;

            SDL_SetRenderDrawColor(renderer, mForegroundColor.r, mForegroundColor.g, mForegroundColor.b, mForegroundColor.a);
            SDL_RenderFillRect(renderer, &fgRect);
        }
    }
}

void ProgressBarComponent::SetProgress(const float progress) {
    mProgress = Math::Max(0.0f, Math::Min(progress, 1.0f));
}

void ProgressBarComponent::SetColors(SDL_Color background, SDL_Color foreground) {
    mBackgroundColor = background;
    mForegroundColor = foreground;
}

void ProgressBarComponent::SetSize(const int width, const int height) {
    mWidth = width;
    mHeight = height;
}
