//
// Created by nslop on 16/04/2024.
//

#include "DrawSpriteComponent.h"
#include "../../Scenes/Scene.h"
#include "../../Actors/Actor.h"
#include "../../Game.h"

DrawSpriteComponent::DrawSpriteComponent(class Actor *owner, const std::string &texturePath, const int width, const int height, const int drawOrder)
        : DrawComponent(owner, drawOrder),
        mWidth(width),
        mHeight(height)
{
    mSpriteSheetSurface = GetScene()->GetGame()->LoadTexture(texturePath);
}

void DrawSpriteComponent::Draw(SDL_Renderer *renderer)
{
    auto srcrect = new SDL_Rect{0,0,mWidth, mHeight};
    int posx = GetOwner()->GetPosition().x - GetOwner()->GetScene()->GetCameraPos().x - mWidth/2;
    int posy = GetOwner()->GetPosition().y - GetOwner()->GetScene()->GetCameraPos().y - mHeight/2;
    auto dstrect = new SDL_Rect {posx, posy, mWidth, mHeight};

    auto flipflag = SDL_RendererFlip::SDL_FLIP_NONE;

    if(GetOwner()->GetRotation() == Math::Pi)
        flipflag = SDL_RendererFlip::SDL_FLIP_HORIZONTAL;

    SDL_RenderCopyEx(renderer, mSpriteSheetSurface, srcrect, dstrect,
                     NULL, nullptr, flipflag);
}