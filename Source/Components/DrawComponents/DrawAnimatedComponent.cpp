//
// Created by nslop on 27/08/2024.
//


#include <fstream>
#include "../../Game.h"
#include "../../Json.h"
#include "../../Actors/Actor.h"
#include "DrawAnimatedComponent.h"

DrawAnimatedComponent::DrawAnimatedComponent(class Actor* owner, const std::string &spriteSheetPath, const std::string &spriteSheetData, int drawOrder)
        :DrawSpriteComponent(owner, spriteSheetPath, 0, 0, drawOrder)
{
    LoadSpriteSheet(spriteSheetPath, spriteSheetData);


}

DrawAnimatedComponent::~DrawAnimatedComponent()
{
    for (const auto& rect : mSpriteSheetData)
    {
        delete rect;
    }
    mSpriteSheetData.clear();
}

void DrawAnimatedComponent::LoadSpriteSheet(const std::string& texturePath, const std::string& dataPath)
{
    // Load sprite sheet texture
    mSpriteSheetSurface = mOwner->GetScene()->GetGame()->LoadTexture(texturePath);

    // Load sprite sheet data
    std::ifstream spriteSheetFile(dataPath);
    nlohmann::json spriteSheetData = nlohmann::json::parse(spriteSheetFile);

    SDL_Rect* rect = nullptr;
    for(const auto& frame : spriteSheetData["frames"]) {

        int x = frame["frame"]["x"].get<int>();
        int y = frame["frame"]["y"].get<int>();
        int w = frame["frame"]["w"].get<int>();
        int h = frame["frame"]["h"].get<int>();
        rect = new SDL_Rect({x, y, w, h});

        mSpriteSheetData.emplace_back(rect);
    }

}

void DrawAnimatedComponent::Draw(SDL_Renderer *renderer) {

    int spriteIdx = mAnimations[mAnimName][(int)mAnimTimer];

    // Is the texture in the map?
    if(spriteIdx < mSpriteSheetData.size())
    {
        Vector2 pos = mOwner->GetPosition();
        Vector2 cameraPos = mOwner->GetScene()->GetGame()->GetCameraPos();

        SDL_Rect *clipRect = mSpriteSheetData[spriteIdx];
        SDL_Rect renderQuad = {static_cast<int>(pos.x - clipRect->w/2.0f - cameraPos.x),
                               static_cast<int>(pos.y - clipRect->h/2.0f - cameraPos.y),
                               clipRect->w,
                               clipRect->h};

        SDL_RendererFlip flip = SDL_FLIP_NONE;

        SDL_RenderCopyEx(renderer, mSpriteSheetSurface, clipRect, &renderQuad, .0f, nullptr, flip);
    }

    auto flipflag = SDL_RendererFlip::SDL_FLIP_NONE;

    if (GetOwner()->GetRotation() == Math::Pi)
        flipflag = SDL_RendererFlip::SDL_FLIP_HORIZONTAL;
}


void DrawAnimatedComponent::Update(float deltaTime)
{
    if(mIsPaused) return;
    mAnimTimer += mAnimFPS * deltaTime;

    while((int)mAnimTimer >= mAnimations[mAnimName].size()) {
        mAnimTimer -= (float)mAnimations[mAnimName].size();
    }
}

void DrawAnimatedComponent::SetAnimation(const std::string& name)
{
    mAnimName = name;
    Update(0);
}

void DrawAnimatedComponent::AddAnimation(const std::string& name, const std::vector<int>& spriteNums)
{
    mAnimations.emplace(name, spriteNums);
}