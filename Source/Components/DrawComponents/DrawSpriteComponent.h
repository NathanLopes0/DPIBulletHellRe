//
// Created by nslop on 16/04/2024.
//

#pragma once

#include "DrawComponent.h"
#include <string>

class DrawSpriteComponent : public DrawComponent {

public:
    DrawSpriteComponent(Actor* owner, const std::string &texturePath, int width, int height, int drawOrder = 100);
    DrawSpriteComponent(Actor* owner, const std::string &texturePath, int drawOrder = 100);

    void Draw(SDL_Renderer* renderer) override;
    [[nodiscard]] int GetSpriteWidth() const { return mWidth; }
    [[nodiscard]] int GetSpriteHeight() const { return mHeight; }

    void SetColor(Uint8 red, Uint8 green, Uint8 blue);

protected:
    SDL_Texture* mSpriteSheetSurface;

    int mWidth;
    int mHeight;
};
