//
// Segmentos de reta desenhados: ponteiros, buscas e chamadas no campo.
//

#pragma once

#include <vector>
#include <SDL_pixels.h>

#include "DrawComponent.h"
#include "../../Motor/Math.h"

/// Desenha uma lista de segmentos, cada um com a sua cor, em coordenadas de tela - e nao relativas
/// ao dono, como os outros desenhos.
class DrawLinhasComponent : public DrawComponent {
public:
    struct Linha {
        Vector2 de;
        Vector2 ate;
        SDL_Color cor;
    };

    DrawLinhasComponent(class Actor* owner, int drawOrder);
    void Draw(SDL_Renderer* renderer) override;

    void Limpar() { mLinhas.clear(); }
    void Acrescentar(const Vector2& de, const Vector2& ate, const SDL_Color cor) {
        mLinhas.push_back({de, ate, cor});
    }
    [[nodiscard]] size_t Quantas() const { return mLinhas.size(); }

private:
    std::vector<Linha> mLinhas;
};
