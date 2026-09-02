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
    // Aplica o color mod DESTE componente na textura, imediatamente antes de
    // desenhar. Necessario porque as texturas agora vem do cache do Game e sao
    // COMPARTILHADAS: chamar SDL_SetTextureColorMod uma vez tingiria todo mundo
    // que usa a mesma imagem. Guardamos a cor por componente e aplicamos no Draw.
    void ApplyColorMod() const;

    SDL_Texture* mSpriteSheetSurface;

    int mWidth;
    int mHeight;

    Uint8 mColorR = 255;
    Uint8 mColorG = 255;
    Uint8 mColorB = 255;
};
