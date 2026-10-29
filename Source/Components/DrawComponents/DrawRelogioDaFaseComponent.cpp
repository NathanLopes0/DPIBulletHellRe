//
// O relogio da fase: a barrinha de tempo que anda colada no professor.
//

#include "DrawRelogioDaFaseComponent.h"

#include <algorithm>
#include <cmath>

#include "../../Actors/Actor.h"
#include "../../Hud.h"
#include "Pincel.h"

using namespace Pincel;

namespace {
    /// Quanto dura o enchimento da troca de fase.
    constexpr float kEnchendo = 0.35f;
}

DrawRelogioDaFaseComponent::DrawRelogioDaFaseComponent(Actor* owner, const int largura, const int altura,
                                                       const int drawOrder)
    : DrawComponent(owner, drawOrder), mLargura(largura), mAltura(altura) {
}

void DrawRelogioDaFaseComponent::SetTempo(const float restante, const float total) {
    mRestante = std::max(0.0f, restante);
    mTotal = std::max(0.001f, total);
}

void DrawRelogioDaFaseComponent::Recomecar() { mDesdeORecomeco = 0.0f; }

void DrawRelogioDaFaseComponent::Update(const float deltaTime) {
    mTempo += deltaTime;
    mDesdeORecomeco += deltaTime;
}

void DrawRelogioDaFaseComponent::Draw(SDL_Renderer* renderer) {
    if (!mIsVisible) return;
    const Misturando misturando(renderer);

    const Vector2 dono = mOwner->GetPosition();
    const int x = static_cast<int>(dono.x), y = static_cast<int>(dono.y);
    const int w = mLargura, h = mAltura;
    const int raio = w / 2;

    float fracao = std::clamp(mRestante / mTotal, 0.0f, 1.0f);
    // NA TROCA DE FASE ela sobe de vazia a cheia, em vez de saltar.
    const float enchendo = std::clamp(mDesdeORecomeco / kEnchendo, 0.0f, 1.0f);
    fracao = std::min(fracao, enchendo);
    const Hud::Rgb rgb = Hud::CorDoRelogio(fracao);
    const SDL_Color cor{rgb.r, rgb.g, rgb.b, 255};

    // 1. O HALO dos ultimos segundos.
    const bool urgente = Hud::Urgente(mRestante);
    if (urgente) {
        const float pulso = 0.5f + 0.5f * std::sin(mTempo * 14.0f);
        for (int k = 3; k >= 1; --k) {
            CaixaLisa(renderer, x - 2 - k, y - 2 - k, w + 4 + 2 * k, h + 4 + 2 * k, raio + 2 + k,
                      SDL_Color{255, 60, 50, static_cast<Uint8>(255.0f * (0.08f + 0.18f * pulso) * static_cast<float>(4 - k) / 3.0f)});
        }
    }

    // 2. A MOLDURA e o SULCO.
    Caixa(renderer, x - 2, y - 2, w + 4, h + 4, raio + 2,
          [](const float t) { return Misturar({110, 118, 150, 255}, {50, 54, 74, 255}, t); });
    CaixaLisa(renderer, x, y, w, h, raio, SDL_Color{8, 9, 15, 255});

    // 3. O QUE FALTA, de baixo para cima, com volume da esquerda para a direita.
    const int cheio = static_cast<int>(std::lround(static_cast<float>(h) * fracao));
    if (cheio > 0) {
        const SDL_Color claro = Clarear(cor, 0.45f);
        const SDL_Color escuro = Escurecer(cor, 0.30f);
        for (int i = h - cheio; i < h; ++i) {
            const int recuo = Recuo(i, h, raio);
            for (int j = recuo; j < w - recuo; ++j) {
                const float t = static_cast<float>(j) / static_cast<float>(std::max(1, w - 1));
                Cor(renderer, t < 0.35f ? Misturar(claro, cor, t / 0.35f) : Misturar(cor, escuro, (t - 0.35f) / 0.65f));
                SDL_RenderDrawPoint(renderer, x + j, y + i);
            }
        }
        // A SUPERFICIE ACESA, onde o tempo esta.
        const int topo = y + h - cheio;
        const int recuo = Recuo(h - cheio, h, raio);
        Cor(renderer, Clarear(cor, 0.8f));
        SDL_RenderDrawLine(renderer, x + recuo, topo, x + w - 1 - recuo, topo);
    }

    // 4. OS DENTES, a cada 5 s: riscos escuros por cima.
    for (const float m : Hud::Marcas(mTotal)) {
        const int my = y + h - static_cast<int>(std::lround(static_cast<float>(h) * m));
        Cor(renderer, SDL_Color{8, 9, 15, 200});
        SDL_RenderDrawLine(renderer, x + 1, my, x + w - 2, my);
        Cor(renderer, SDL_Color{140, 150, 190, 255}, 0.8f);
        SDL_RenderDrawLine(renderer, x - 3, my, x - 1, my);
    }

    // 5. O CLARAO de quando a fase troca.
    if (mDesdeORecomeco < kEnchendo + 0.25f) {
        const float a = 1.0f - mDesdeORecomeco / (kEnchendo + 0.25f);
        CaixaLisa(renderer, x - 2, y - 2, w + 4, h + 4, raio + 2, SDL_Color{255, 255, 255, static_cast<Uint8>(150.0f * a)});
    }
}
