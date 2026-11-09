//
// O fundo do campo da batalha.
//

#include "DrawFundoDoCampoComponent.h"

#include <cmath>
#include <SDL.h>

#include "../../Interface/FundoDoCampo.h"
#include "../../Motor/EscalaDaTela.h"

DrawFundoDoCampoComponent::DrawFundoDoCampoComponent(Actor* owner, const SDL_FRect campo, const int drawOrder)
    : DrawComponent(owner, drawOrder),
      mCampo(campo)
{
}

DrawFundoDoCampoComponent::~DrawFundoDoCampoComponent() {
    if (mTextura) SDL_DestroyTexture(mTextura);
}

void DrawFundoDoCampoComponent::Draw(SDL_Renderer* renderer) {
    if (!IsVisible()) return;

    const float escala = EscalaDaTela(renderer);
    if (!mTextura || std::fabs(escala - mEscala) > 0.001f) {
        if (mTextura) SDL_DestroyTexture(mTextura);
        mTextura = nullptr;
        mEscala = escala;

        const int largura = static_cast<int>(std::lround(mCampo.w * escala));
        const int altura = static_cast<int>(std::lround(mCampo.h * escala));
        const auto pixels = FundoDoCampo::Pintar(largura, altura, escala);
        if (pixels.empty()) return;

        mTextura = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, largura, altura);
        if (!mTextura) {
            SDL_Log("FUNDO: nao consegui criar a textura do fundo (%dx%d): %s", largura, altura, SDL_GetError());
            return;
        }
        SDL_UpdateTexture(mTextura, nullptr, pixels.data(), largura * static_cast<int>(sizeof(std::uint32_t)));
    }

    SDL_RenderCopyF(renderer, mTextura, nullptr, &mCampo);
}
