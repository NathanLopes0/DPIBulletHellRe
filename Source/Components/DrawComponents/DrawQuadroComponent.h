//
// O quadro da sala: o placar da batalha, de giz.
//

#pragma once

#include "DrawComponent.h"

/// O quadro-negro do canto da batalha, onde o professor escreve o que esta fazendo.
class DrawQuadroComponent : public DrawComponent {
public:
    DrawQuadroComponent(class Actor* owner, int largura, int altura, int drawOrder);

    void Update(float deltaTime) override;
    void Draw(SDL_Renderer* renderer) override;

    /// O texto trocou: o apagador passa.
    void Apagar() { mDesdeOApagador = 0.0f; }

private:
    int mLargura;
    int mAltura;
    float mDesdeOApagador = 10.0f;
};
