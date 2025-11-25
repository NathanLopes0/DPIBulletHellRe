//
// Created by nslop on 04/06/2024.
//

#include "DrawTextComponent.h"
#include "../../Font.h"
#include "../../Actors/Actor.h"
#include "../../Game.h"
#include "../../Scenes/Scene.h"

DrawTextComponent::DrawTextComponent(Actor *owner, const std::string &text, Font *font, int width, int height, int fontSize, int drawOrder)
    :DrawComponent(owner, drawOrder),
    mWidth(width),
    mHeight(height),
    mSize(fontSize),
    mFont(font)
{
    mTextSurface = mFont->RenderText(owner->GetScene()->GetGame()->GetRenderer(), text,
                                     Vector3(1.0, 1.0, 1.0), fontSize, 500);
}

DrawTextComponent::~DrawTextComponent()
{
    SDL_DestroyTexture(mTextSurface);
}

void DrawTextComponent::SetText(const std::string &text)
{
    SDL_DestroyTexture(mTextSurface);
    mTextSurface = mFont->RenderText(mOwner->GetScene()->GetGame()->GetRenderer(), text, Vector3(1.0, 1.0, 1.0), mSize, 500);
}

void DrawTextComponent::Draw(SDL_Renderer *renderer)
{
    if (!IsVisible()) return;
    Vector2 pos = mOwner->GetPosition();

    SDL_Rect renderQuad = {static_cast<int>(pos.x - mWidth/2.0f) ,
                           static_cast<int>(pos.y - mHeight/2.0f),
                           mWidth,
                           mHeight};

    SDL_RenderCopyEx(renderer, mTextSurface, nullptr, &renderQuad, .0f, nullptr, SDL_FLIP_NONE);
}