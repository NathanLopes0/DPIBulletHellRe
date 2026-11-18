//
// O modo daltonismo na tela.
//

#include "Daltonizar.h"

#include <cstdint>
#include <unordered_map>

#include "../Interface/Daltonismo.h"

namespace Daltonizar {

    void NaTela(Uint8& r, Uint8& g, Uint8& b) {
        const Daltonismo::Modo m = Daltonismo::Atual();
        if (m == Daltonismo::Modo::Desligado) return;
        const Daltonismo::Rgb c = Daltonismo::Corrigir({r, g, b}, m);
        r = c.r;
        g = c.g;
        b = c.b;
    }

    SDL_Color NaTela(SDL_Color c) {
        NaTela(c.r, c.g, c.b);
        return c;
    }

    void CorDeDesenho(SDL_Renderer* renderer, Uint8 r, Uint8 g, Uint8 b, const Uint8 a) {
        NaTela(r, g, b);
        SDL_SetRenderDrawColor(renderer, r, g, b, a);
    }

    void Tinto(SDL_Texture* textura, Uint8 r, Uint8 g, Uint8 b) {
        NaTela(r, g, b);
        SDL_SetTextureColorMod(textura, r, g, b);
    }

    void Superficie(SDL_Surface*& s) {
        const Daltonismo::Modo m = Daltonismo::Atual();
        if (!s || m == Daltonismo::Modo::Desligado) return;
        SDL_Surface* rgba = SDL_ConvertSurfaceFormat(s, SDL_PIXELFORMAT_RGBA32, 0);
        if (!rgba) return;   // fica a original: melhor sem correcao que sem imagem
        SDL_FreeSurface(s);
        s = rgba;

        std::unordered_map<std::uint32_t, Daltonismo::Rgb> feitas;
        if (SDL_LockSurface(s) != 0) return;
        for (int y = 0; y < s->h; ++y) {
            auto* linha = static_cast<Uint8*>(s->pixels) + y * s->pitch;
            for (int x = 0; x < s->w; ++x) {
                Uint8* p = linha + 4 * x;
                if (p[3] == 0) continue;
                const std::uint32_t chave = (static_cast<std::uint32_t>(p[0]) << 16) |
                                            (static_cast<std::uint32_t>(p[1]) << 8) | p[2];
                auto it = feitas.find(chave);
                if (it == feitas.end()) it = feitas.emplace(chave, Daltonismo::Corrigir({p[0], p[1], p[2]}, m)).first;
                p[0] = it->second.r;
                p[1] = it->second.g;
                p[2] = it->second.b;
            }
        }
        SDL_UnlockSurface(s);
    }
}
