//
// Created by nslop on 16/04/2024.
//

#pragma once

#include "DrawComponent.h"
#include <string>

class DrawSpriteComponent : public DrawComponent {

public:
    DrawSpriteComponent(class Actor* owner, const std::string &texturePath, int width = 0, int height = 0, int drawOrder = 100);

    void Draw(SDL_Renderer* renderer) override;
    [[nodiscard]] int GetSpriteWidth() const { return mWidth; }
    [[nodiscard]] int GetSpriteHeight() const { return mHeight; }

protected:
    SDL_Texture* mSpriteSheetSurface;

    int mWidth;
    int mHeight;
};
