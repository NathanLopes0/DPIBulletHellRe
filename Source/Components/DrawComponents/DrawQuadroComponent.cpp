//
// O quadro da sala: o placar da batalha, de giz.
//

#include "DrawQuadroComponent.h"

#include <algorithm>
#include <cmath>

#include "../../Actors/Actor.h"
#include "Pincel.h"

using namespace Pincel;

namespace {
    constexpr int kMadeira = 4;
    constexpr int kCanaleta = 4;
    constexpr float kApagador = 0.32f;

    constexpr SDL_Color kMadeiraClara{168, 116, 70, 255};
    constexpr SDL_Color kMadeiraEscura{96, 62, 36, 255};
    /// TRANSLUCIDA, como era: o campo continua aparecendo atras.
    constexpr SDL_Color kLousaCima{30, 68, 48, 205};
    constexpr SDL_Color kLousaBaixo{18, 46, 32, 215};
}

DrawQuadroComponent::DrawQuadroComponent(Actor* owner, const int largura, const int altura, const int drawOrder)
    : DrawComponent(owner, drawOrder), mLargura(largura), mAltura(altura) {
}

void DrawQuadroComponent::Update(const float deltaTime) { mDesdeOApagador += deltaTime; }

void DrawQuadroComponent::Draw(SDL_Renderer* renderer) {
    if (!mIsVisible) return;
    const Misturando misturando(renderer);

    const Vector2 c = mOwner->GetPosition();
    const int w = mLargura, h = mAltura;
    const int x = static_cast<int>(c.x) - w / 2, y = static_cast<int>(c.y) - h / 2;

    // 1. A MOLDURA DE MADEIRA, clara em cima e escura embaixo, e a sombra no campo.
    CaixaLisa(renderer, x - kMadeira + 3, y - kMadeira + 4, w + 2 * kMadeira, h + 2 * kMadeira + kCanaleta, 5,
              SDL_Color{0, 0, 0, 90});
    Caixa(renderer, x - kMadeira, y - kMadeira, w + 2 * kMadeira, h + 2 * kMadeira + kCanaleta, 5,
          [](const float t) { return Misturar(kMadeiraClara, kMadeiraEscura, t); });

    // 2. A LOUSA.
    Caixa(renderer, x, y, w, h, 2, [](const float t) { return Misturar(kLousaCima, kLousaBaixo, t); });
    // O PO DE GIZ: borroes de apagador velhos, quase invisiveis.
    Cor(renderer, SDL_Color{230, 240, 230, 255}, 0.07f);
    for (int k = 0; k < 3; ++k) {
        const int bx = x + 14 + k * (w / 3), by = y + 6 + (k % 2) * (h / 3);
        for (int i = 0; i < 10; ++i) SDL_RenderDrawLine(renderer, bx + i * 3, by + i, bx + i * 3 + 26, by + i - 4);
    }
    // O FIO DE LUZ no alto da lousa, onde a moldura faz sombra.
    Cor(renderer, SDL_Color{0, 0, 0, 255}, 0.35f);
    SDL_RenderDrawLine(renderer, x, y, x + w - 1, y);

    // 3. A CANALETA, e o toco de giz nela.
    const int cy = y + h + kMadeira - 1;
    Cor(renderer, Clarear(kMadeiraClara, 0.2f));
    SDL_RenderDrawLine(renderer, x - kMadeira + 2, cy, x + w + kMadeira - 3, cy);
    CaixaLisa(renderer, x + w - 30, cy - 3, 14, 4, 2, SDL_Color{240, 240, 230, 255});

    // 4. O APAGADOR, quando o texto troca: uma faixa clara que varre a lousa.
    if (mDesdeOApagador < kApagador) {
        const float u = mDesdeOApagador / kApagador;
        const int px = x + static_cast<int>(static_cast<float>(w + 60) * u) - 30;
        for (int k = -30; k <= 30; ++k) {
            const int xx = px + k;
            if (xx < x || xx >= x + w) continue;
            const float a = (1.0f - std::fabs(static_cast<float>(k)) / 30.0f) * (1.0f - u * 0.5f);
            Cor(renderer, SDL_Color{220, 235, 220, 255}, 0.28f * a);
            SDL_RenderDrawLine(renderer, xx, y + 1, xx, y + h - 2);
        }
    }
}
