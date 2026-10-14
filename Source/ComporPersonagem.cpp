//
// Monta as camadas da personagem numa textura unica.
//

#include "ComporPersonagem.h"

#include <SDL.h>
#include <SDL_image.h>

#include "CaminhosArquivo.h"
#include "Game.h"

namespace {

    /// Multiplica cada pixel pela cor escolhida, preservando o alfa.
    ///
    /// A conta por canal vive em Personagens::Multiplicar, que e pura e testada;
    /// aqui so ha a varredura. O alfa NAO entra na multiplicacao: tingir a
    /// transparencia comeria as bordas da peca a cada composicao.
    void Tingir(SDL_Surface* s, const unsigned char r, const unsigned char g,
                const unsigned char b) {

        if (SDL_MUSTLOCK(s) && SDL_LockSurface(s) != 0) return;

        for (int y = 0; y < s->h; ++y) {

            auto* linha = reinterpret_cast<Uint32*>(
                static_cast<Uint8*>(s->pixels) + static_cast<size_t>(y) * s->pitch);

            for (int x = 0; x < s->w; ++x) {
                Uint8 pr, pg, pb, pa;
                SDL_GetRGBA(linha[x], s->format, &pr, &pg, &pb, &pa);
                if (pa == 0) continue;

                linha[x] = SDL_MapRGBA(s->format,
                                       Personagens::Multiplicar(pr, r),
                                       Personagens::Multiplicar(pg, g),
                                       Personagens::Multiplicar(pb, b),
                                       pa);
            }
        }

        if (SDL_MUSTLOCK(s)) SDL_UnlockSurface(s);
    }

    /// Abre um PNG ja no formato em que a conta de tingir funciona.
    ///
    /// A conversao nao e luxo: um PNG pode vir indexado ou sem canal alfa, e a
    /// varredura acima trata todo pixel como 32 bits. Sem converter, uma peca
    /// salva em formato diferente das outras sairia com as cores embaralhadas.
    SDL_Surface* AbrirComo32(const std::string& caminho) {

        SDL_Surface* bruta = IMG_Load(caminho.c_str());
        if (bruta == nullptr) {
            SDL_Log("PERSONAGEM: nao consegui abrir a camada %s (%s)",
                    caminho.c_str(), IMG_GetError());
            return nullptr;
        }

        SDL_Surface* convertida = SDL_ConvertSurfaceFormat(bruta, SDL_PIXELFORMAT_RGBA32, 0);
        SDL_FreeSurface(bruta);

        if (convertida == nullptr) {
            SDL_Log("PERSONAGEM: nao consegui converter %s (%s)",
                    caminho.c_str(), SDL_GetError());
        }
        return convertida;
    }
}

namespace Personagens {

Composta Compor(Game* game, const Catalogo& catalogo, const Aparencia& aparencia) {

    Composta saida;

    if (game == nullptr || game->GetRenderer() == nullptr) return saida;

    const std::vector<Camada> camadas = catalogo.Camadas(aparencia);
    if (camadas.empty()) {
        SDL_Log("PERSONAGEM: esta aparencia nao tem camada nenhuma; nada a compor.");
        return saida;
    }

    // A chave sai da aparencia JA RESOLVIDA, e nao da pedida: duas aparencias
    // que diferem so num cabelo que nao existe mais sao a mesma personagem na
    // tela, e devem reaproveitar a mesma textura.
    saida.chaveDaTextura = ChaveDaAparencia(catalogo.Resolver(aparencia));
    saida.atlas = Caminhos::Asset(camadas.front().arte + ".json");

    if (game->TexturaGuardada(saida.chaveDaTextura) != nullptr) {
        saida.ok = true;
        return saida;
    }

    SDL_Surface* destino = nullptr;

    for (const auto& camada : camadas) {

        SDL_Surface* peca = AbrirComo32(Caminhos::Asset(camada.arte + ".png"));
        if (peca == nullptr) continue;   // ja relatado; as outras camadas seguem

        if (camada.tingida) Tingir(peca, camada.r, camada.g, camada.b);

        if (destino == nullptr) {
            // O tamanho vem da PRIMEIRA camada que abriu, e nao de um numero
            // fixo aqui: assim trocar a arte por uma de outra resolucao e so
            // trocar os arquivos.
            destino = SDL_CreateRGBSurfaceWithFormat(0, peca->w, peca->h, 32,
                                                     SDL_PIXELFORMAT_RGBA32);
            if (destino == nullptr) {
                SDL_Log("PERSONAGEM: nao consegui criar a superficie de destino (%s)",
                        SDL_GetError());
                SDL_FreeSurface(peca);
                return saida;
            }
            SDL_FillRect(destino, nullptr, SDL_MapRGBA(destino->format, 0, 0, 0, 0));
        }
        else if (peca->w != destino->w || peca->h != destino->h) {
            SDL_Log("PERSONAGEM: a camada %s tem %dx%d e as outras tem %dx%d; ela vai sair "
                    "desalinhada. Todas as pecas precisam do mesmo recorte.",
                    camada.arte.c_str(), peca->w, peca->h, destino->w, destino->h);
        }

        // Mistura, e nao copia: sem isto a camada de cima apagaria o que havia
        // atras dela, inclusive onde ela e transparente - sobraria so o cabelo.
        SDL_SetSurfaceBlendMode(peca, SDL_BLENDMODE_BLEND);
        SDL_BlitSurface(peca, nullptr, destino, nullptr);
        SDL_FreeSurface(peca);
    }

    if (destino == nullptr) {
        SDL_Log("PERSONAGEM: nenhuma camada pode ser aberta; a personagem nao foi composta.");
        return saida;
    }

    SDL_Texture* textura = SDL_CreateTextureFromSurface(game->GetRenderer(), destino);
    SDL_FreeSurface(destino);

    if (textura == nullptr) {
        SDL_Log("PERSONAGEM: nao consegui criar a textura composta (%s)", SDL_GetError());
        return saida;
    }

    game->GuardarTextura(saida.chaveDaTextura, textura);
    saida.ok = true;
    return saida;
}

}
