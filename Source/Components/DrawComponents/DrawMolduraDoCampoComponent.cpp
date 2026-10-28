//
// A moldura do campo da batalha: as bordas onde o aluno nao vai.
//

#include "DrawMolduraDoCampoComponent.h"

#include <algorithm>
#include <cmath>

#include "Pincel.h"

using namespace Pincel;

namespace {
    // O METAL ESCURO da HUD: o mesmo azul-noite das placas da faixa da nota.
    constexpr SDL_Color kFora{5, 6, 11, 255};
    constexpr SDL_Color kDentro{22, 24, 38, 255};
    constexpr SDL_Color kTrilho{74, 80, 110, 255};
    constexpr SDL_Color kFio{150, 160, 205, 255};
    constexpr SDL_Color kEncaixe{196, 170, 230, 255};
    constexpr SDL_Color kEncosto{150, 220, 255, 255};

    /// O comprimento do trecho da parede que acende perto do aluno.
    constexpr float kTrechoDoEncosto = 150.0f;
    /// A profundidade do clarao, para dentro do campo.
    constexpr int kFundoDoClarao = 14;
    /// Quantos pixels a luz do encosto entra no campo.
    constexpr int kEspessuraDoEncosto = 6;
}

DrawMolduraDoCampoComponent::DrawMolduraDoCampoComponent(Actor* owner, const int drawOrder)
    : DrawComponent(owner, drawOrder) {
}

void DrawMolduraDoCampoComponent::Definir(const float largura, const float altura, const Hud::Caixa& area) {
    mLargura = largura;
    mAltura = altura;
    mArea = area;
}

void DrawMolduraDoCampoComponent::Acender(const SDL_Color cor, const float duracao) {
    // O MAIS FORTE GANHA: um Ponto Extra no meio de um acerto nao apaga o
    // vermelho que acabou de acender.
    if (Hud::Clarao(mIdadeDoClarao, mDuracaoDoClarao) > 0.6f && duracao < mDuracaoDoClarao) return;
    mCorDoClarao = cor;
    mIdadeDoClarao = 0.0f;
    mDuracaoDoClarao = duracao;
}

void DrawMolduraDoCampoComponent::Update(const float deltaTime) {
    mTempo += deltaTime;
    if (mIdadeDoClarao >= 0.0f) mIdadeDoClarao += deltaTime;
}

void DrawMolduraDoCampoComponent::DesenharTrilho(SDL_Renderer* r, const int x0, const int x1, const int y0, const int y1,
                                                 const bool esquerdo) const {
    if (x1 <= x0) return;
    // ESCURO POR FORA, um pouco mais claro por dentro: o trilho tem fundo.
    for (int x = x0; x < x1; ++x) {
        const float t = static_cast<float>(x - x0) / static_cast<float>(std::max(1, x1 - x0 - 1));
        Cor(r, Misturar(kFora, kDentro, esquerdo ? t : 1.0f - t));
        SDL_RenderDrawLine(r, x, y0, x, y1);
    }
    // A BORDA DE DENTRO: o trilho claro e o fio de luz.
    const int borda = esquerdo ? x1 - 1 : x0;
    const int fio = esquerdo ? x1 - 3 : x0 + 2;
    Cor(r, kTrilho);
    SDL_RenderDrawLine(r, borda, y0, borda, y1);
    Cor(r, kTrilho, 0.7f);
    SDL_RenderDrawLine(r, esquerdo ? borda - 1 : borda + 1, y0, esquerdo ? borda - 1 : borda + 1, y1);
    Cor(r, kFio, 0.35f);
    SDL_RenderDrawLine(r, fio, y0, fio, y1);
    // OS REBITES, a cada 64 px: o trilho e uma peca, e nao uma faixa pintada.
    for (int y = y0 + 40; y < y1 - 20; y += 64) {
        const int cx = (x0 + x1) / 2 - (esquerdo ? 1 : -1);
        Cor(r, kTrilho, 0.9f);
        SDL_RenderDrawLine(r, cx - 1, y, cx + 1, y);
        SDL_RenderDrawLine(r, cx, y - 1, cx, y + 1);
    }
}

void DrawMolduraDoCampoComponent::DesenharPainel(SDL_Renderer* r) const {
    const int topo = static_cast<int>(mArea.baixo);
    const int w = static_cast<int>(mLargura);
    const int h = static_cast<int>(mAltura);
    // O PAINEL: claro em cima, escuro embaixo.
    for (int y = topo; y < h; ++y) {
        const float t = static_cast<float>(y - topo) / static_cast<float>(std::max(1, h - topo - 1));
        Cor(r, Misturar(kDentro, kFora, t));
        SDL_RenderDrawLine(r, 0, y, w - 1, y);
    }
    // A BORDA DE CIMA, a mesma dos trilhos, com o fio de luz por baixo.
    Cor(r, kTrilho);
    SDL_RenderDrawLine(r, 0, topo, w - 1, topo);
    Cor(r, kTrilho, 0.7f);
    SDL_RenderDrawLine(r, 0, topo + 1, w - 1, topo + 1);
    Cor(r, kFio, 0.35f);
    SDL_RenderDrawLine(r, 0, topo + 3, w - 1, topo + 3);
}

void DrawMolduraDoCampoComponent::DesenharEncostos(SDL_Renderer* r) const {
    const auto acender = [&](const float forca, const bool vertical, const float fixo, const float ao, const float de,
                             const float ate, const int paraDentro) {
        if (forca <= 0.0f) return;
        // UM TRECHO DA PAREDE, mais forte no ponto mais perto do aluno, e com
        // espessura: a luz entra alguns pixels no campo.
        const float meio = kTrechoDoEncosto / 2.0f;
        for (float s = -meio; s <= meio; s += 1.0f) {
            const float p = ao + s;
            if (p < de || p > ate) continue;
            const float queda = 1.0f - std::fabs(s) / meio;
            const float a = forca * queda * queda;
            for (int k = 0; k < kEspessuraDoEncosto; ++k) {
                Cor(r, k == 0 ? Clarear(kEncosto, 0.5f) : kEncosto,
                    a * (1.0f - static_cast<float>(k) / kEspessuraDoEncosto));
                const int f = static_cast<int>(fixo) + paraDentro * k, q = static_cast<int>(p);
                if (vertical) SDL_RenderDrawPoint(r, f, q);
                else SDL_RenderDrawPoint(r, q, f);
            }
        }
    };
    for (const auto& [centro, meio] : mAlunos) {
        const Hud::Encosto e = Hud::Encostado(mArea, centro, meio);
        acender(e.esquerda, true, mArea.esquerda, centro.y, mArea.topo, mArea.baixo, 1);
        acender(e.direita, true, mArea.direita, centro.y, mArea.topo, mArea.baixo, -1);
        acender(e.baixo, false, mArea.baixo - 1.0f, centro.x, mArea.esquerda, mArea.direita, -1);
        acender(e.topo, false, mArea.topo, centro.x, mArea.esquerda, mArea.direita, 1);
    }
}

void DrawMolduraDoCampoComponent::DesenharClarao(SDL_Renderer* r) const {
    const float c = Hud::Clarao(mIdadeDoClarao, mDuracaoDoClarao);
    if (c <= 0.0f) return;
    // O CLARAO ENTRA PELAS BORDAS do campo e some para o meio.
    const int e = static_cast<int>(mArea.esquerda), d = static_cast<int>(mArea.direita);
    const int b = static_cast<int>(mArea.baixo), t = 0;
    for (int k = 0; k < kFundoDoClarao; ++k) {
        const float a = c * 0.55f * (1.0f - static_cast<float>(k) / kFundoDoClarao);
        Cor(r, mCorDoClarao, a);
        SDL_RenderDrawLine(r, e + k, t, e + k, b - k);
        SDL_RenderDrawLine(r, d - k, t, d - k, b - k);
        SDL_RenderDrawLine(r, e + k, b - k, d - k, b - k);
    }
}

void DrawMolduraDoCampoComponent::Draw(SDL_Renderer* renderer) {
    if (!mIsVisible || mLargura <= 0.0f) return;
    const Misturando misturando(renderer);

    const int topoDoPainel = static_cast<int>(mArea.baixo);
    DesenharClarao(renderer);
    DesenharTrilho(renderer, 0, static_cast<int>(mArea.esquerda), 0, topoDoPainel, true);
    DesenharTrilho(renderer, static_cast<int>(mArea.direita) + 1, static_cast<int>(mLargura), 0, topoDoPainel, false);
    DesenharPainel(renderer);

    // EM CIMA, SO UMA LINHA tracejada, e bem apagada: e o limite do aluno,
    // mas e o lugar do professor.
    Cor(renderer, kFio, 0.16f);
    for (int x = static_cast<int>(mArea.esquerda) + 6; x < static_cast<int>(mArea.direita) - 6; x += 14) {
        SDL_RenderDrawLine(renderer, x, static_cast<int>(mArea.topo), x + 6, static_cast<int>(mArea.topo));
    }

    // OS ENCAIXES, onde os trilhos encontram o painel: um losango lilas.
    for (const float x : {mArea.esquerda, mArea.direita}) {
        const int cx = static_cast<int>(x), cy = topoDoPainel;
        for (int k = 0; k <= 5; ++k) {
            Cor(renderer, kEncaixe, k == 5 ? 0.6f : 0.9f);
            SDL_RenderDrawLine(renderer, cx - (5 - k), cy - k, cx + (5 - k), cy - k);
            SDL_RenderDrawLine(renderer, cx - (5 - k), cy + k, cx + (5 - k), cy + k);
        }
        Cor(renderer, kDentro);
        SDL_RenderDrawPoint(renderer, cx, cy);
    }

    DesenharEncostos(renderer);
}
