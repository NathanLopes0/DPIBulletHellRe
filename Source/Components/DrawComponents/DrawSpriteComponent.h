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

    /// Uma escala so deste desenho, multiplicada pela do ator: uma arte feita no dobro do tamanho
    /// desenha em 0.5, e o resto do ator (o texto) nao muda.
    void SetEscalaPropria(const float escala) { mEscalaPropria = escala; }

protected:
    // Aplica o color mod deste componente na textura, imediatamente antes de desenhar.
    void ApplyColorMod() const;

    SDL_Texture* mSpriteSheetSurface;

    int mWidth;
    int mHeight;

    Uint8 mColorR = 255;
    Uint8 mColorG = 255;
    Uint8 mColorB = 255;

    float mEscalaPropria = 1.0f;
};
