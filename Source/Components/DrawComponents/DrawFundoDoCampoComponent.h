//
// O fundo do campo da batalha (FundoDoCampo.h), desenhado na resolucao da tela.
//

#pragma once

#include <SDL_rect.h>

#include "DrawComponent.h"

/// Pinta o fundo uma vez numa textura do tamanho real da tela, e de novo so quando a tela muda.
class DrawFundoDoCampoComponent : public DrawComponent {
public:
    DrawFundoDoCampoComponent(class Actor* owner, SDL_FRect campo, int drawOrder);
    ~DrawFundoDoCampoComponent() override;

    void Draw(SDL_Renderer* renderer) override;

private:
    SDL_FRect mCampo;
    struct SDL_Texture* mTextura = nullptr;
    float mEscala = 0.0f;
};
