//
// Created by nslop on 16/04/2024.
//

#include "DrawSpriteComponent.h"
#include "../../Scenes/Scene.h"
#include "../../Actors/Actor.h"
#include "../../Game.h"

// Construtor quando você quer especificar o tamanho da imagem
DrawSpriteComponent::DrawSpriteComponent(class Actor *owner, const std::string &texturePath, const int width, const int height, const int drawOrder)
        : DrawComponent(owner, drawOrder),
        mWidth(width),
        mHeight(height)
{
    mSpriteSheetSurface = GetScene()->GetGame()->LoadTexture(texturePath);
}

// Construtor quando você quer usar o tamanho original da imagem
DrawSpriteComponent::DrawSpriteComponent(Actor *owner, const std::string &texturePath, int drawOrder)
    : DrawComponent(owner, drawOrder),
    mWidth(0),
    mHeight(0)
{
    mSpriteSheetSurface = GetScene()->GetGame()->LoadTexture(texturePath);
    if (mSpriteSheetSurface) {
        SDL_QueryTexture(mSpriteSheetSurface, nullptr, nullptr, &mWidth, &mHeight);
    }
}


void DrawSpriteComponent::Draw(SDL_Renderer *renderer)
{

    float scale = mOwner->GetScale();

    int scaledWidth = static_cast<int>(mWidth * scale);
    int scaledHeight = static_cast<int>(mHeight * scale);


    SDL_Rect srcrect = {0,0,mWidth, mHeight};
    int posx = static_cast<int>(GetOwner()->GetPosition().x - GetOwner()->GetScene()->GetGame()->GetCameraPos().x - scaledWidth / 2.0f);
    int posy = static_cast<int>(GetOwner()->GetPosition().y - GetOwner()->GetScene()->GetGame()->GetCameraPos().y - scaledHeight / 2.0f);

    SDL_Rect dstrect = {posx, posy, scaledWidth, scaledHeight};

    auto flipflag = SDL_RendererFlip::SDL_FLIP_NONE;

    if(GetOwner()->GetRotation() == Math::Pi)
        flipflag = SDL_RendererFlip::SDL_FLIP_HORIZONTAL;

    SDL_RenderCopyEx(renderer, mSpriteSheetSurface, &srcrect, &dstrect,
                     0.0, nullptr, flipflag);
}
