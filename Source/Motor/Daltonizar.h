//
// O modo daltonismo na tela: onde o SDL pinta, a cor passa por Daltonismo::Corrigir.
//

#pragma once

#include <SDL.h>

/// A ponte entre Daltonismo (puro) e o SDL. Nao ha filtro na tela inteira: cada caminho por onde uma
/// cor chega a tela - a imagem carregada, a cor de desenho das linhas e caixas, o tinto de um sprite
/// e o texto - corrige a sua, uma vez, no modo atual.
namespace Daltonizar {

    /// A cor no modo atual (a opacidade nao muda).
    SDL_Color NaTela(SDL_Color c);
    void NaTela(Uint8& r, Uint8& g, Uint8& b);

    /// SDL_SetRenderDrawColor, com a cor corrigida.
    void CorDeDesenho(SDL_Renderer* renderer, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
    /// SDL_SetTextureColorMod, com a cor corrigida.
    void Tinto(SDL_Texture* textura, Uint8 r, Uint8 g, Uint8 b);

    /// A imagem recem-lida, corrigida pixel a pixel antes de virar textura. Desligado, nada muda; ligado,
    /// 's' passa a ser uma copia em RGBA32 (a de antes e liberada). As artes tem poucas cores, entao
    /// cada cor e calculada uma vez.
    void Superficie(SDL_Surface*& s);
}
