//
// O pincel da HUD: os tracos que a barra da nota, a moldura do campo, o relogio
// da fase e o quadro desenham do mesmo jeito.
//

#pragma once

#include <algorithm>
#include <cmath>
#include <SDL.h>

/**
 * UM LUGAR SO para as primitivas da HUD: misturar cores, clarear, escurecer, e
 * a caixa de pontas redondas pintada linha a linha (com a cor de cada linha
 * dada por quem chama - e assim que sai o volume, claro em cima e escuro
 * embaixo). Antes, cada desenho da HUD tinha a sua copia, e copias assim
 * acabam ajustadas uma de cada vez ate deixarem de combinar.
 *
 * Quem chama poe o renderer em SDL_BLENDMODE_BLEND antes (e devolve depois).
 */
namespace Pincel {

    inline SDL_Color Misturar(const SDL_Color a, const SDL_Color b, const float t) {
        const float u = std::clamp(t, 0.0f, 1.0f);
        const auto m = [u](const Uint8 x, const Uint8 y) {
            return static_cast<Uint8>(static_cast<float>(x) + (static_cast<float>(y) - static_cast<float>(x)) * u);
        };
        return {m(a.r, b.r), m(a.g, b.g), m(a.b, b.b), m(a.a, b.a)};
    }
    inline SDL_Color Clarear(const SDL_Color c, const float t) { return Misturar(c, {255, 255, 255, c.a}, t); }
    inline SDL_Color Escurecer(const SDL_Color c, const float t) { return Misturar(c, {0, 0, 0, c.a}, t); }

    /// A cor de desenho, com a opacidade da cor multiplicada por 'alfa'.
    inline void Cor(SDL_Renderer* r, const SDL_Color c, const float alfa = 1.0f) {
        SDL_SetRenderDrawColor(r, c.r, c.g, c.b,
                               static_cast<Uint8>(std::clamp(static_cast<float>(c.a) * alfa, 0.0f, 255.0f)));
    }

    /// Quanto a linha 'i' de uma caixa de 'altura' recua das pontas, com raio 'raio'.
    inline int Recuo(const int i, const int altura, const int raio) {
        const int r = std::min(raio, altura / 2);
        const int d = i < r ? r - i : (i >= altura - r ? i - (altura - r - 1) : 0);
        if (d <= 0) return 0;
        const float dy = static_cast<float>(d) - 0.5f;
        return static_cast<int>(std::lround(static_cast<float>(r) -
                                            std::sqrt(std::max(0.0f, static_cast<float>(r * r) - dy * dy))));
    }

    /// Uma caixa de pontas redondas; 'tom(t)' da a cor da linha (t de 0 em cima a 1 embaixo).
    template <typename Tom>
    void Caixa(SDL_Renderer* r, const int x, const int y, const int w, const int h, const int raio, Tom tom) {
        for (int i = 0; i < h; ++i) {
            const int recuo = Recuo(i, h, raio);
            if (w - 2 * recuo <= 0) continue;
            Cor(r, tom(static_cast<float>(i) / static_cast<float>(std::max(1, h - 1))));
            SDL_RenderDrawLine(r, x + recuo, y + i, x + w - 1 - recuo, y + i);
        }
    }

    /// Uma caixa de pontas redondas de uma cor so.
    inline void CaixaLisa(SDL_Renderer* r, const int x, const int y, const int w, const int h, const int raio,
                          const SDL_Color cor) {
        Caixa(r, x, y, w, h, raio, [cor](float) { return cor; });
    }

    /// A mesma caixa, deitada: 'tom(t)' da a cor da COLUNA (t de 0 a esquerda a 1 a direita).
    template <typename Tom>
    void CaixaDeitada(SDL_Renderer* r, const int x, const int y, const int w, const int h, const int raio, Tom tom) {
        for (int j = 0; j < w; ++j) {
            const int recuo = Recuo(j, w, raio);
            if (h - 2 * recuo <= 0) continue;
            Cor(r, tom(static_cast<float>(j) / static_cast<float>(std::max(1, w - 1))));
            SDL_RenderDrawLine(r, x + j, y + recuo, x + j, y + h - 1 - recuo);
        }
    }

    /// Guarda e devolve o modo de mistura: quem desenha com transparencia usa
    /// isto no comeco do Draw.
    class Misturando {
    public:
        explicit Misturando(SDL_Renderer* r) : mR(r) {
            SDL_GetRenderDrawBlendMode(r, &mAntes);
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        }
        ~Misturando() { SDL_SetRenderDrawBlendMode(mR, mAntes); }
        Misturando(const Misturando&) = delete;
        Misturando& operator=(const Misturando&) = delete;

    private:
        SDL_Renderer* mR;
        SDL_BlendMode mAntes{};
    };
}
