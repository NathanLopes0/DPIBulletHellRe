//
// Created by gensh on 11/11/2025.
//

#include "ProgressBarComponent.h"
#include "../../Actors/Actor.h"

ProgressBarComponent::ProgressBarComponent(Actor* owner, const int drawOrder)
    : DrawComponent(owner, drawOrder)
{
}

void ProgressBarComponent::Draw(SDL_Renderer* renderer) {

    if (!mIsVisible) {
        return;
    }

    const Vector2 ownerPos = mOwner->GetPosition();
    const int posX = static_cast<int>(ownerPos.x);
    const int posY = static_cast<int>(ownerPos.y);

    // 1.
    const SDL_Rect bgRect{posX, posY, mWidth, mHeight};
    SDL_SetRenderDrawColor(renderer, mBackgroundColor.r, mBackgroundColor.g, mBackgroundColor.b, mBackgroundColor.a);
    SDL_RenderFillRect(renderer, &bgRect);
    DesenharSulco(renderer, bgRect);

    // 2.
    const SDL_Color cor = LerpColor(mStartColor, mEndColor, mColorPercent);
    const int cheio = static_cast<int>(static_cast<float>(mWidth) * mFillPercent);
    if (cheio <= 0) return;

    SDL_SetRenderDrawColor(renderer, cor.r, cor.g, cor.b, cor.a);
    const SDL_Rect fgRect{posX, posY, cheio, mHeight};
    SDL_RenderFillRect(renderer, &fgRect);

    // 3.
    DesenharBrilho(renderer, fgRect, cor);
}

void ProgressBarComponent::SetFillPercent(const float percent) {
    mFillPercent = Math::Max(0.0f, Math::Min(percent, 1.0f));
}

void ProgressBarComponent::SetColorPercent(const float percent) {
    mColorPercent = Math::Max(0.0f, Math::Min(percent, 1.0f));
}

void ProgressBarComponent::SetGradientColors(SDL_Color background, SDL_Color start, SDL_Color end) {
    mBackgroundColor = background;
    mStartColor = start;
    mEndColor = end;
}

void ProgressBarComponent::SetSize(const int width, const int height) {
    mWidth = width;
    mHeight = height;
}

SDL_Color ProgressBarComponent::Clarear(const SDL_Color cor, const float quanto) {
    return LerpColor(cor, {255, 255, 255, cor.a}, quanto);
}

SDL_Color ProgressBarComponent::Escurecer(const SDL_Color cor, const float quanto) {
    return LerpColor(cor, {0, 0, 0, cor.a}, quanto);
}

/// Quanto do sulco e sombra, e quao acesa fica a ponta.
namespace {
    constexpr int kEspessuraDaMoldura = 2;
    constexpr int kAlturaDaSombra     = 2;   ///< o risco escuro no topo do sulco
    constexpr int kAlturaDoBrilho     = 2;   ///< o risco claro no topo do cheio
    constexpr int kLarguraDaPonta     = 4;   ///< a faixa acesa na ponta do cheio

    constexpr float kClaroDoBrilho  = 0.45f;
    constexpr float kClaroDaPonta   = 0.70f;
    constexpr float kEscuroDaSombra = 0.50f;

    /// A moldura clareia o fundo, e nao escurece.
    constexpr float kClaroDaMoldura = 0.30f;
}

void ProgressBarComponent::DesenharSulco(SDL_Renderer* renderer, const SDL_Rect& caixa) const {

    // A moldura fecha a barra.
    const SDL_Color moldura = Clarear(mBackgroundColor, kClaroDaMoldura);
    SDL_SetRenderDrawColor(renderer, moldura.r, moldura.g, moldura.b, 255);
    for (int i = 0; i < kEspessuraDaMoldura; ++i) {
        SDL_Rect anel{caixa.x - i - 1, caixa.y - i - 1, caixa.w + 2 * (i + 1), caixa.h + 2 * (i + 1)};
        SDL_RenderDrawRect(renderer, &anel);
    }

    // A sombra no topo de dentro: da ao vazio a leitura de buraco, e nao de outra cor chapada.
    const SDL_Color sombra = Escurecer(mBackgroundColor, kEscuroDaSombra);
    SDL_SetRenderDrawColor(renderer, sombra.r, sombra.g, sombra.b, 255);
    SDL_Rect risco{caixa.x, caixa.y, caixa.w, kAlturaDaSombra};
    SDL_RenderFillRect(renderer, &risco);
}

void ProgressBarComponent::DesenharBrilho(SDL_Renderer* renderer, const SDL_Rect& preenchido,
                                          const SDL_Color cor) const {

    if (preenchido.w <= 0 || preenchido.h <= 0) return;

    // O brilho no topo do cheio, do mesmo tom em claro.
    const SDL_Color claro = Clarear(cor, kClaroDoBrilho);
    SDL_SetRenderDrawColor(renderer, claro.r, claro.g, claro.b, cor.a);
    SDL_Rect topo{preenchido.x, preenchido.y, preenchido.w, kAlturaDoBrilho};
    SDL_RenderFillRect(renderer, &topo);

    // A ponta acesa.
    if (preenchido.w <= kLarguraDaPonta) return;

    const SDL_Color ponta = Clarear(cor, kClaroDaPonta);
    SDL_SetRenderDrawColor(renderer, ponta.r, ponta.g, ponta.b, cor.a);
    SDL_Rect fim{preenchido.x + preenchido.w - kLarguraDaPonta, preenchido.y,
                 kLarguraDaPonta, preenchido.h};
    SDL_RenderFillRect(renderer, &fim);
}

SDL_Color ProgressBarComponent::LerpColor(const SDL_Color colorA, const SDL_Color colorB, float alpha) {
    alpha = Math::Clamp(alpha, 0.0f, 1.0f);
    return {
        static_cast<Uint8>(static_cast<float>(colorA.r) + (static_cast<float>(colorB.r) - static_cast<float>(colorA.r)) * alpha),
        static_cast<Uint8>(static_cast<float>(colorA.g) + (static_cast<float>(colorB.g) - static_cast<float>(colorA.g)) * alpha),
        static_cast<Uint8>(static_cast<float>(colorA.b) + (static_cast<float>(colorB.b) - static_cast<float>(colorA.b)) * alpha),
        static_cast<Uint8>(static_cast<float>(colorA.a) + (static_cast<float>(colorB.a) - static_cast<float>(colorA.a)) * alpha)
    };
}
