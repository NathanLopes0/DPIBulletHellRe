//
// Created by nslop on 04/06/2024.
//

#include "DrawTextComponent.h"

#include <algorithm>
#include <cmath>
#include "../../Motor/EscalaDaTela.h"
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
    SDL_Renderer* renderer = owner->GetScene()->GetGame()->GetRenderer();
    mEscala = EscalaDaTela(renderer);
    mTextSurface = mFont->RenderText(renderer, text, mCor, fontSize, mLarguraDeQuebra, mEscala);
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
                                     mCor, mSize, mLarguraDeQuebra, mEscala);
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

    // A tela mudou de tamanho: o texto e desenhado de novo, nitido no tamanho novo.
    const float escalaDaTela = EscalaDaTela(renderer);
    if (std::fabs(escalaDaTela - mEscala) > 0.001f) {
        mEscala = escalaDaTela;
        Redesenhar();
    }

    const Vector2 pos = mOwner->GetPosition();

    auto largura = static_cast<float>(mWidth);
    auto altura = static_cast<float>(mHeight);

    if (mAjustarAoTexto && mTextSurface) {

        int natW = 0, natH = 0;
        if (SDL_QueryTexture(mTextSurface, nullptr, nullptr, &natW, &natH) == 0
            && natW > 0 && natH > 0) {

            // O tamanho natural, em pixels do jogo.
            const float w = static_cast<float>(natW) / mEscala;
            const float h = static_cast<float>(natH) / mEscala;

            // So encolhe, e mantendo a proporcao: um texto que nao cabe fica menor, nunca deformado.
            const float encolhe = std::min(1.0f, std::min(largura / w, altura / h));
            largura = w * encolhe;
            altura  = h * encolhe;
        }
    }

    // O canto cai num pixel inteiro da tela: no tamanho natural, cada pixel da textura e um da tela.
    const auto noPixel = [this](const float v) { return std::round(v * mEscala) / mEscala; };
    const float esquerda = mAEsquerda ? pos.x - static_cast<float>(mWidth) / 2.0f : pos.x - largura / 2.0f;
    const SDL_FRect renderQuad = {noPixel(esquerda), noPixel(pos.y - altura / 2.0f), largura, altura};

    SDL_RenderCopyExF(renderer, mTextSurface, nullptr, &renderQuad, 0.0, nullptr, SDL_FLIP_NONE);
}