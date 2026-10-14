//
// Created by gensh on 11/11/2025.
//

#include "ProgressBarComponent.h"
#include "../../Actors/Actor.h"
#include "SDL.h"

ProgressBarComponent::ProgressBarComponent(Actor* owner, int drawOrder)
    // NA ORDEM EM QUE OS MEMBROS SAO DECLARADOS. O compilador inicializa nessa
    // ordem de qualquer jeito, e a lista fora de ordem so dava um aviso
    // (-Wreorder-ctor) que escondia os avisos de verdade neste arquivo.
    : DrawComponent(owner, drawOrder)
    , mFillPercent(0.0f)
    , mColorPercent(0.0f)
    , mWidth(200)
    , mHeight(200)
    , mBackgroundColor({50,50,50, 255})
    , mStartColor({255, 100, 150, 255})
    , mEndColor({0,0,0,0})
    , mUseGradient(false)
    , mEffect(VisualEffect::None)
    , mEffectTimer(0.0f)
    , mOrientation(Orientation::Horizontal)
{

}

void ProgressBarComponent::Update(float deltaTime) {
    if (mEffect == VisualEffect::Pulsing) {
        mEffectTimer += deltaTime;
    }
}


void ProgressBarComponent::Draw(SDL_Renderer* renderer) {

    if (!mIsVisible) {
        return;
    }

    Vector2 ownerPos = mOwner->GetPosition();
    const int posX = static_cast<int>(ownerPos.x);
    const int posY = static_cast<int>(ownerPos.y);

    // 1. Desenha o Fundo (sempre igual)
    SDL_Rect bgRect;
    bgRect.x = posX;
    bgRect.y = posY;
    bgRect.w = mWidth;
    bgRect.h = mHeight;
    SDL_SetRenderDrawColor(renderer, mBackgroundColor.r, mBackgroundColor.g, mBackgroundColor.b, mBackgroundColor.a);
    SDL_RenderFillRect(renderer, &bgRect);

    // 1b. A moldura e o sulco, se esta barra pediu acabamento.
    if (mAcabamento) DesenharSulco(renderer, bgRect);

    // 2. Desenha o Preenchimento
    if (mUseGradient) {
        // --- LÓGICA DE COR SÓLIDA INTERPOLADA ---

        // 1. Calcular a cor-alvo ÚNICA baseada no *percentual de cor*
        SDL_Color targetColor = LerpColor(mStartColor, mEndColor, mColorPercent);

        // 2. Aplicar efeitos visuais
        if (mEffect == VisualEffect::Pulsing) {
            float pulse = (sinf(mEffectTimer * 8.0f) + 1.0f) / 2.0f;
            float alphaMod = pulse;
            targetColor.a = static_cast<Uint8>(static_cast<float>(targetColor.a) * alphaMod);
        }

        // 3. Definir a cor de desenho para essa cor calculada.
        SDL_SetRenderDrawColor(renderer, targetColor.r, targetColor.g, targetColor.b, targetColor.a);

        // 4. Desenhar um ÚNICO retângulo de preenchimento (baseado no *percentual de preenchimento*)
        SDL_Rect fgRect;
        fgRect.x = posX;
        fgRect.y = posY;

        if (mOrientation == Orientation::Horizontal) {
            if (const int fgWidth = static_cast<int>(static_cast<float>(mWidth) * mFillPercent); fgWidth > 0) {
                fgRect.w = fgWidth;
                fgRect.h = mHeight;
                SDL_RenderFillRect(renderer, &fgRect);

                // 3. O brilho vem DEPOIS do preenchimento: ele e o acabamento
                // dele, e desenhado antes ficaria por baixo.
                if (mAcabamento) DesenharBrilho(renderer, fgRect, targetColor);
            }
        } else { // Vertical
            if (const int fgHeight = static_cast<int>(static_cast<float>(mHeight) * mFillPercent); fgHeight > 0) {
                const int emptyHeight = mHeight - fgHeight;
                fgRect.w = mWidth;
                fgRect.h = fgHeight;
                fgRect.y = posY + emptyHeight;
                SDL_RenderFillRect(renderer, &fgRect);
            }
        }

    } else {
        // --- COR SÓLIDA ---
        SDL_SetRenderDrawColor(renderer, mStartColor.r, mStartColor.g, mStartColor.b, mStartColor.a);

        SDL_Rect fgRect;
        fgRect.x = posX;
        fgRect.y = posY;

        if (mOrientation == Orientation::Horizontal) {
            if (const int fgWidth = static_cast<int>(static_cast<float>(mWidth) * mFillPercent); fgWidth > 0) {
                fgRect.w = fgWidth;
                fgRect.h = mHeight;
                SDL_RenderFillRect(renderer, &fgRect);
            }
        } else { // Vertical
            if (const int fgHeight = static_cast<int>(static_cast<float>(mHeight) * mFillPercent); fgHeight > 0) {
                const int emptyHeight = mHeight - fgHeight;
                fgRect.w = mWidth;
                fgRect.h = fgHeight;
                fgRect.y = posY + emptyHeight;
                SDL_RenderFillRect(renderer, &fgRect);
            }
        }
    }
}

void ProgressBarComponent::SetFillPercent(const float percent) {
    mFillPercent = Math::Max(0.0f, Math::Min(percent, 1.0f));
}

void ProgressBarComponent::SetColorPercent(const float percent) {
    mColorPercent = Math::Max(0.0f, Math::Min(percent, 1.0f));
}

void ProgressBarComponent::SetColors(SDL_Color background, SDL_Color foreground) {
    mBackgroundColor = background;
    mStartColor = foreground;
    mUseGradient = false;
}

void ProgressBarComponent::SetGradientColors(SDL_Color background, SDL_Color start, SDL_Color end) {
    mBackgroundColor = background;
    mStartColor = start;
    mEndColor = end;
    mUseGradient = true;
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

/// Quanto do sulco e sombra, e quao acesa fica a ponta. Em UM lugar: sao tres
/// desenhos que tem de combinar entre si, e tres numeros soltos pelo Draw
/// acabariam ajustados um de cada vez ate deixarem de combinar.
namespace {
    constexpr int kEspessuraDaMoldura = 2;
    constexpr int kAlturaDaSombra     = 2;   ///< o risco escuro no topo do sulco
    constexpr int kAlturaDoBrilho     = 2;   ///< o risco claro no topo do cheio
    constexpr int kLarguraDaPonta     = 4;   ///< a faixa acesa na ponta do cheio

    constexpr float kClaroDoBrilho  = 0.45f;
    constexpr float kClaroDaPonta   = 0.70f;
    constexpr float kEscuroDaSombra = 0.50f;

    /// A moldura CLAREIA o fundo, e nao escurece. O sulco ja e quase preto, e
    /// uma moldura mais escura que ele desapareceria no preto da tela - a barra
    /// continuaria sem contorno, que e justamente o que a moldura veio resolver.
    constexpr float kClaroDaMoldura = 0.30f;
}

void ProgressBarComponent::DesenharSulco(SDL_Renderer* renderer, const SDL_Rect& caixa) const {

    // A MOLDURA fecha a barra. Sem ela o retangulo encosta no preto da tela e
    // nao se sabe onde a barra acaba e a tela comeca - era o que fazia a faixa
    // parecer um risco, e nao uma peca.
    const SDL_Color moldura = Clarear(mBackgroundColor, kClaroDaMoldura);
    SDL_SetRenderDrawColor(renderer, moldura.r, moldura.g, moldura.b, 255);
    for (int i = 0; i < kEspessuraDaMoldura; ++i) {
        SDL_Rect anel{caixa.x - i - 1, caixa.y - i - 1, caixa.w + 2 * (i + 1), caixa.h + 2 * (i + 1)};
        SDL_RenderDrawRect(renderer, &anel);
    }

    // A SOMBRA no topo de dentro: da ao vazio a leitura de buraco, e nao de
    // outra cor chapada. E o unico motivo de a parte vazia parecer funda.
    const SDL_Color sombra = Escurecer(mBackgroundColor, kEscuroDaSombra);
    SDL_SetRenderDrawColor(renderer, sombra.r, sombra.g, sombra.b, 255);
    SDL_Rect risco{caixa.x, caixa.y, caixa.w, kAlturaDaSombra};
    SDL_RenderFillRect(renderer, &risco);
}

void ProgressBarComponent::DesenharBrilho(SDL_Renderer* renderer, const SDL_Rect& preenchido,
                                          const SDL_Color cor) const {

    if (preenchido.w <= 0 || preenchido.h <= 0) return;

    // O BRILHO NO TOPO do cheio, do mesmo tom em claro. Duas linhas bastam: e o
    // que separa "retangulo pintado" de "coisa com volume".
    const SDL_Color claro = Clarear(cor, kClaroDoBrilho);
    SDL_SetRenderDrawColor(renderer, claro.r, claro.g, claro.b, cor.a);
    SDL_Rect topo{preenchido.x, preenchido.y, preenchido.w, kAlturaDoBrilho};
    SDL_RenderFillRect(renderer, &topo);

    // A PONTA ACESA. O corte do preenchimento era uma aresta dura; acesa, ela
    // vira o lugar para onde o olho vai - e e exatamente onde esta a nota.
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
