//
// O desenho da barra da nota.
//

#include "DrawBarraDaNotaComponent.h"

#include <algorithm>
#include <cmath>

#include "../../Actors/Actor.h"
#include "Pincel.h"

namespace {
    /// O raio das pontas, em pixels. Nunca mais que meia altura.
    constexpr int kRaio = 8;
    /// A moldura em volta do sulco.
    constexpr int kMoldura = 3;
    /// O reflexo que atravessa o preenchimento: a cada tanto, a esta rapidez.
    constexpr float kEntreReflexos = 3.2f;
    constexpr float kRapidezDoReflexo = 900.0f;
    constexpr int kLarguraDoReflexo = 46;

    constexpr SDL_Color kAmbar{255, 190, 70, 255};
    constexpr SDL_Color kVerdeDaAprovacao{120, 235, 150, 255};
    constexpr SDL_Color kVermelhoDoPerigo{255, 60, 60, 255};
    constexpr SDL_Color kOuro{255, 215, 80, 255};
}

// As primitivas sao as da HUD inteira (Pincel.h).
using namespace Pincel;

DrawBarraDaNotaComponent::DrawBarraDaNotaComponent(Actor* owner, const int largura, const int altura, const int drawOrder)
    : DrawComponent(owner, drawOrder), mLargura(largura), mAltura(altura), mLarguraDoRodape(largura) {
}

DrawBarraDaNotaComponent::~DrawBarraDaNotaComponent() {
    if (mTextura) SDL_DestroyTexture(mTextura);
}

void DrawBarraDaNotaComponent::DefinirPose(const int largura, const float angulo, const bool espelhada) {
    mPosada = true;
    mLargura = std::max(2 * kRaio + 2, largura);
    mAngulo = angulo;
    mEspelhada = espelhada;
}

void DrawBarraDaNotaComponent::TirarPose() {
    mPosada = false;
    mLargura = mLarguraDoRodape;
    mAngulo = 0.0f;
    mEspelhada = false;
}

void DrawBarraDaNotaComponent::Comecar(const float nota) {
    mEstado = BarraDaNota::Comecar(nota);
    mNota = nota;
}

void DrawBarraDaNotaComponent::SetNota(const float nota, const SDL_Color cor, const bool perigo, const bool cheia) {
    mNota = nota;
    mCor = cor;
    mPerigo = perigo;
    mCheia = cheia;
}

void DrawBarraDaNotaComponent::Update(const float deltaTime) {
    mTempo += deltaTime;
    BarraDaNota::Avancar(mEstado, mNota, deltaTime);
}

void DrawBarraDaNotaComponent::Draw(SDL_Renderer* renderer) {
    if (!mIsVisible) return;

    // O sacolejo do acerto: dois pixels, e so na vertical - a barra nao pode fugir do lugar.
    const int tremor = static_cast<int>(std::lround(std::sin(mTempo * 55.0f) * 2.0f * mEstado.tremor));
    const Vector2 dono = mOwner->GetPosition();
    const bool girada = mPosada && (std::fabs(mAngulo) > 0.01f || mEspelhada);
    if (!girada || !SDL_RenderTargetSupported(renderer)) {
        Desenhar(renderer, static_cast<int>(dono.x), static_cast<int>(dono.y) + tremor);
        return;
    }

    // Girada: desenha deitada numa textura, com folga para o halo e as marcas, e gira a textura em
    // volta do centro da barra.
    constexpr int kFolga = 18;
    const int tw = mLargura + 2 * kFolga, th = mAltura + 2 * kFolga;
    if (!mTextura || tw != mLarguraDaTextura || th != mAlturaDaTextura) {
        if (mTextura) SDL_DestroyTexture(mTextura);
        mTextura = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, tw, th);
        mLarguraDaTextura = tw;
        mAlturaDaTextura = th;
        if (!mTextura) {
            Desenhar(renderer, static_cast<int>(dono.x), static_cast<int>(dono.y) + tremor);
            return;
        }
        SDL_SetTextureBlendMode(mTextura, SDL_BLENDMODE_BLEND);
    }
    SDL_Texture* antes = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, mTextura);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    Desenhar(renderer, kFolga, kFolga + tremor);
    SDL_SetRenderTarget(renderer, antes);
    const SDL_FRect destino{dono.x - static_cast<float>(kFolga), dono.y - static_cast<float>(kFolga),
                            static_cast<float>(tw), static_cast<float>(th)};
    SDL_RenderCopyExF(renderer, mTextura, nullptr, &destino, mAngulo, nullptr,
                      mEspelhada ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}

void DrawBarraDaNotaComponent::Desenhar(SDL_Renderer* renderer, const int x, const int y) {
    const Misturando misturando(renderer);
    const int w = mLargura, h = mAltura;

    // 1.
    if (mPerigo || mCheia) {
        const float pulso = 0.5f + 0.5f * std::sin(mTempo * (mPerigo ? 7.0f : 3.0f));
        const SDL_Color halo = mPerigo ? kVermelhoDoPerigo : kOuro;
        for (int k = 3; k >= 1; --k) {
            const float alfa = (0.10f + 0.22f * pulso) * static_cast<float>(4 - k) / 3.0f;
            const int o = kMoldura + 2 * k;
            Caixa(renderer, x - o, y - o, w + 2 * o, h + 2 * o, kRaio + o,
                  [&](float) { SDL_Color c = halo; c.a = static_cast<Uint8>(255.0f * alfa); return c; });
        }
    }

    // 2.
    Caixa(renderer, x - kMoldura, y - kMoldura, w + 2 * kMoldura, h + 2 * kMoldura, kRaio + kMoldura,
          [](const float t) { return Misturar({96, 104, 136, 255}, {40, 44, 62, 255}, t); });
    Caixa(renderer, x, y, w, h, kRaio,
          [](const float t) { return Misturar({8, 9, 15, 255}, {30, 33, 48, 255}, t); });

    // 3.
    for (const float n : BarraDaNota::kMarcasFinas) {
        const int mx = x + static_cast<int>(BarraDaNota::X(n, w));
        Cor(renderer, {255, 255, 255, 26});
        SDL_RenderDrawLine(renderer, mx, y + 4, mx, y + h - 5);
    }

    // 4.
    const int xNota = static_cast<int>(BarraDaNota::X(mEstado.mostrada, w));
    const int xRastro = static_cast<int>(BarraDaNota::X(mEstado.rastro, w));
    const float vidaDoRastro = mEstado.rastroParado > 0.0f ? 1.0f : 0.75f;
    for (int i = 0; i < h; ++i) {
        const int recuo = Recuo(i, h, kRaio);
        const int de = std::max(x + xNota, x + recuo);
        const int ate = std::min(x + xRastro, x + w - 1 - recuo);
        if (ate <= de) continue;
        Cor(renderer, Misturar({255, 235, 230, 255}, {255, 120, 110, 255}, static_cast<float>(i) / static_cast<float>(h)),
            0.85f * vidaDoRastro);
        SDL_RenderDrawLine(renderer, de, y + i, ate, y + i);
    }

    // 5.
    if (xNota > 0) {
        const SDL_Color base = Clarear(mCor, 0.45f * mEstado.brilho);
        const SDL_Color topo = Clarear(base, 0.50f);
        const SDL_Color fundo = Escurecer(base, 0.35f);
        for (int i = 0; i < h; ++i) {
            const int recuo = Recuo(i, h, kRaio);
            const int de = x + recuo;
            const int ate = std::min(x + xNota, x + w - 1 - recuo);
            if (ate < de) continue;
            const float t = static_cast<float>(i) / static_cast<float>(std::max(1, h - 1));
            Cor(renderer, t < 0.4f ? Misturar(topo, base, t / 0.4f) : Misturar(base, fundo, (t - 0.4f) / 0.6f));
            SDL_RenderDrawLine(renderer, de, y + i, ate, y + i);
            // O reflexo que passa, de tempos em tempos.
            const float fase = std::fmod(mTempo, kEntreReflexos) * kRapidezDoReflexo - static_cast<float>(kLarguraDoReflexo);
            const int r0 = std::max(de, x + static_cast<int>(fase) + i / 2);
            const int r1 = std::min(ate, r0 + kLarguraDoReflexo / 2);
            if (r1 > r0 && t < 0.75f) {
                Cor(renderer, {255, 255, 255, 255}, 0.22f * (1.0f - t));
                SDL_RenderDrawLine(renderer, r0, y + i, r1, y + i);
            }
        }
        // O brilho de cima, uma faixa so.
        const int recuo = Recuo(2, h, kRaio);
        if (x + xNota - 1 > x + recuo) {
            Cor(renderer, {255, 255, 255, 255}, 0.35f);
            SDL_RenderDrawLine(renderer, x + recuo + 1, y + 2, std::min(x + xNota - 2, x + w - 2 - recuo), y + 2);
        }
        // A ponta acesa, onde o olho procura a nota.
        if (xNota > 3 && xNota < w - 2) {
            Cor(renderer, Clarear(base, 0.85f));
            SDL_Rect ponta{x + xNota - 2, y + 1, 2, h - 2};
            SDL_RenderFillRect(renderer, &ponta);
        }
    }

    // 6.
    const auto marca = [&](const float n, const SDL_Color c) {
        const int mx = x + static_cast<int>(BarraDaNota::X(n, w));
        const bool passou = mEstado.mostrada >= n;
        Cor(renderer, c, passou ? 0.55f : 0.9f);
        SDL_Rect linha{mx - 1, y + 1, 2, h - 2};
        SDL_RenderFillRect(renderer, &linha);
        Cor(renderer, c);
        for (int k = 0; k < 5; ++k) {
            SDL_RenderDrawLine(renderer, mx - 4 + k, y - kMoldura - 5 + k, mx + 4 - k, y - kMoldura - 5 + k);
            SDL_RenderDrawLine(renderer, mx - 4 + k, y + h + kMoldura + 4 - k, mx + 4 - k, y + h + kMoldura + 4 - k);
        }
    };
    marca(BarraDaNota::kMarcaDoExame, kAmbar);
    marca(BarraDaNota::kMarcaDaAprovacao, kVerdeDaAprovacao);
}
