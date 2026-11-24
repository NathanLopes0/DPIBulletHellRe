//
// O relogio da fase: a ampulheta que anda colada no professor.
//

#include "DrawRelogioDaFaseComponent.h"

#include <algorithm>
#include <cmath>

#include "../../Actors/Actor.h"
#include "../../Interface/Hud.h"
#include "Pincel.h"

using namespace Pincel;

namespace {
    constexpr SDL_Color kVidro{140, 150, 190, 255};
    constexpr SDL_Color kDentro{14, 16, 26, 235};
    constexpr SDL_Color kMadeira{150, 98, 52, 255};
    constexpr SDL_Color kMadeiraEscura{92, 58, 28, 255};
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
    const float meio = static_cast<float>(w) / 2.0f;
    const float maisLargo = meio - 1.0f;

    // A virada da troca de fase: ela gira meia volta (a escala vertical vai a zero e volta), e a
    // areia, que estava toda embaixo, aparece toda em cima.
    const bool virando = mDesdeORecomeco < Hud::kVirada;
    const float escala = Hud::EscalaDaVirada(mDesdeORecomeco);
    float fracao = std::clamp(mRestante / mTotal, 0.0f, 1.0f);
    if (virando) fracao = Hud::JaVirou(mDesdeORecomeco) ? 1.0f : 0.0f;
    const bool urgente = Hud::Urgente(mRestante) && !virando;
    const float pulso = 0.5f + 0.5f * std::sin(mTempo * 14.0f);
    const Hud::Rgb rgb = Hud::CorDaAreia(fracao);
    SDL_Color areia{rgb.r, rgb.g, rgb.b, 255};
    if (urgente) areia = Misturar(areia, SDL_Color{255, 70, 60, 255}, 0.35f + 0.5f * pulso);

    // 1. O brilho dos ultimos segundos.
    if (urgente) {
        for (int k = 3; k >= 1; --k) {
            CaixaLisa(renderer, x - 2 - k, y - 2 - k, w + 4 + 2 * k, h + 4 + 2 * k, 6 + k,
                      SDL_Color{255, 60, 50, static_cast<Uint8>(255.0f * (0.08f + 0.18f * pulso) * static_cast<float>(4 - k) / 3.0f)});
        }
    }

    // 2. O vidro e a areia, linha a linha: 'v' e a altura no desenho em pe (0 em cima, 1 embaixo).
    const float topoDeCima = 0.5f - 0.5f * Hud::AreiaDeCima(fracao);    // a areia de cima vai daqui ao gargalo
    const float topoDeBaixo = 1.0f - 0.5f * Hud::AreiaDeBaixo(fracao);  // o monte de baixo vai daqui ao fundo
    const float centro = static_cast<float>(h) / 2.0f;
    const float metade = std::max(0.5f, centro * escala);
    for (int r = 0; r < h; ++r) {
        const float d = (static_cast<float>(r) + 0.5f - centro) / metade;   // -1 a 1 na ampulheta
        if (d < -1.0f || d > 1.0f) continue;
        const float v = (d + 1.0f) / 2.0f;
        const int meia = std::max(1, static_cast<int>(std::lround(maisLargo * Hud::MeiaLarguraDoVidro(v))));
        const int esquerda = x + static_cast<int>(std::lround(meio)) - meia;
        const int direita = x + static_cast<int>(std::lround(meio)) + meia - 1;
        const bool temAreia = (v < 0.5f && v >= topoDeCima && fracao > 0.0f) || (v >= 0.5f && v >= topoDeBaixo && fracao < 1.0f);
        Cor(renderer, temAreia ? areia : kDentro);
        if (direita - 1 >= esquerda + 1) SDL_RenderDrawLine(renderer, esquerda + 1, y + r, direita - 1, y + r);
        // A areia de cima e mais clara na superficie; o vidro, nas bordas.
        Cor(renderer, kVidro, 0.9f);
        SDL_RenderDrawPoint(renderer, esquerda, y + r);
        SDL_RenderDrawPoint(renderer, direita, y + r);
    }
    // O fio de areia que cai pelo gargalo, tracejado e andando.
    if (!virando && fracao > 0.0f && fracao < 1.0f) {
        const int cx = x + static_cast<int>(std::lround(meio)) - 1;
        const int de = y + static_cast<int>(centro);
        const int ate = y + static_cast<int>(centro + metade * (2.0f * topoDeBaixo - 1.0f));
        const int passo = static_cast<int>(mTempo * 30.0f) % 4;
        Cor(renderer, Clarear(areia, 0.3f));
        for (int yy = de; yy < ate; ++yy) {
            if ((yy - de + passo) % 4 < 2) SDL_RenderDrawPoint(renderer, cx, yy);
        }
    }

    // 3. As duas tampas de madeira, que acompanham a virada.
    for (const float lado : {-1.0f, 1.0f}) {
        const int cy = y + static_cast<int>(std::lround(centro + lado * metade));
        Caixa(renderer, x - 2, cy - 2, w + 4, 4, 1,
              [](const float t) { return Misturar(kMadeira, kMadeiraEscura, t); });
    }

    // 4. O clarao do fim da virada.
    if (mDesdeORecomeco < Hud::kVirada + 0.25f) {
        const float a = std::clamp(1.0f - (mDesdeORecomeco - Hud::kVirada * 0.5f) / (Hud::kVirada * 0.5f + 0.25f), 0.0f, 1.0f);
        if (a > 0.0f) CaixaLisa(renderer, x - 2, y - 2, w + 4, h + 4, 6, SDL_Color{255, 255, 255, static_cast<Uint8>(90.0f * a)});
    }
}
