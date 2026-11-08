//
// Created by nslop on 04/06/2024.
//

#include "DrawTextComponent.h"

#include <algorithm>
#include "../../Motor/Font.h"
#include "../../Actors/Actor.h"
#include "../../Motor/Game.h"
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
                                     mCor, fontSize, mLarguraDeQuebra);
}

DrawTextComponent::~DrawTextComponent()
{
    SDL_DestroyTexture(mTextSurface);
}

void DrawTextComponent::SetText(const std::string &text)
{
    // O texto igual nao e redesenhado (padrao Dirty Flag).
    if (text == mTexto) return;
    mTexto = text;
    Redesenhar();
}

void DrawTextComponent::Redesenhar()
{
    SDL_DestroyTexture(mTextSurface);
    mTextSurface = mFont->RenderText(mOwner->GetScene()->GetGame()->GetRenderer(), mTexto,
                                     mCor, mSize, mLarguraDeQuebra);
}

void DrawTextComponent::SetLarguraDeQuebra(const unsigned largura)
{
    if (largura == mLarguraDeQuebra) return;
    mLarguraDeQuebra = largura;
    Redesenhar();
}

void DrawTextComponent::SetColor(const Vector3& cor)
{
    if (cor.x == mCor.x && cor.y == mCor.y && cor.z == mCor.z) return;

    mCor = cor;
    Redesenhar();
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

            // So encolhe, e mantendo a proporcao: um texto que nao cabe fica menor, nunca deformado.
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