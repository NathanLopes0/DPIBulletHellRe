//
// Created by nslop on 04/06/2024.
//

#include "DrawTextComponent.h"

#include <algorithm>
#include "../../Font.h"
#include "../../Actors/Actor.h"
#include "../../Game.h"
#include "../../Scenes/Scene.h"

DrawTextComponent::DrawTextComponent(Actor *owner, const std::string &text, Font *font, int width, int height, int fontSize, int drawOrder)
    :DrawComponent(owner, drawOrder),
    mWidth(width),
    mHeight(height),
    mSize(fontSize),
    mFont(font),
    mTexto(text)
{
    mTextSurface = mFont->RenderText(owner->GetScene()->GetGame()->GetRenderer(), text,
                                     Vector3(1.0, 1.0, 1.0), fontSize, mLarguraDeQuebra);
}

DrawTextComponent::~DrawTextComponent()
{
    SDL_DestroyTexture(mTextSurface);
}

void DrawTextComponent::SetText(const std::string &text)
{
    mTexto = text;
    SDL_DestroyTexture(mTextSurface);
    mTextSurface = mFont->RenderText(mOwner->GetScene()->GetGame()->GetRenderer(), text,
                                     Vector3(1.0, 1.0, 1.0), mSize, mLarguraDeQuebra);
}

void DrawTextComponent::SetLarguraDeQuebra(const unsigned largura)
{
    if (largura == mLarguraDeQuebra) return;
    mLarguraDeQuebra = largura;
    SetText(mTexto);
}

void DrawTextComponent::Draw(SDL_Renderer *renderer)
{
    if (!IsVisible()) return;
    Vector2 pos = mOwner->GetPosition();

    int largura = mWidth;
    int altura = mHeight;

    if (mAjustarAoTexto && mTextSurface) {

        int natW = 0, natH = 0;
        if (SDL_QueryTexture(mTextSurface, nullptr, nullptr, &natW, &natH) == 0
            && natW > 0 && natH > 0) {

            largura = natW;
            altura = natH;

            // So ENCOLHE, e mantendo a proporcao: um texto que nao cabe fica menor,
            // nunca deformado. Esticar para preencher e justamente o que esta
            // opcao existe para evitar.
            const float escala = std::min(1.0f,
                                          std::min(static_cast<float>(mWidth) / static_cast<float>(natW),
                                                   static_cast<float>(mHeight) / static_cast<float>(natH)));
            largura = static_cast<int>(static_cast<float>(natW) * escala);
            altura  = static_cast<int>(static_cast<float>(natH) * escala);
        }
    }

    SDL_Rect renderQuad = {static_cast<int>(pos.x - largura / 2.0f),
                           static_cast<int>(pos.y - altura / 2.0f),
                           largura,
                           altura};

    SDL_RenderCopyEx(renderer, mTextSurface, nullptr, &renderQuad, .0f, nullptr, SDL_FLIP_NONE);
}