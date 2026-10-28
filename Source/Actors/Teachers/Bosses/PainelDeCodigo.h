//
// Paineis e linhas de codigo desenhados no campo.
//

#pragma once

#include <string>

#include <SDL_pixels.h>

#include "../../Actor.h"

class DrawCaixaComponent;
class DrawTextComponent;

/**
 * @class PainelDeCodigo
 * @brief Um retangulo com fundo e moldura, centrado no ator: um bloco do
 *        programa escrito no campo.
 *
 * E SO DESENHO: nao tem colisor, nao e projetil, nao tem tempo de vida. Quem
 * o cria (o professor) o destroi quando a fase dele acaba, e o esconde quando
 * a batalha termina.
 */
class PainelDeCodigo : public Actor {
public:
    PainelDeCodigo(Scene* cena, int largura, int altura, int ordem);

    void Pintar(SDL_Color moldura, SDL_Color fundo);
    void Mostrar(bool mostrar);

private:
    DrawCaixaComponent* mCaixa = nullptr;
};

/**
 * @class EtiquetaDeCodigo
 * @brief Uma linha de codigo numa caixinha, na fonte do placar: a declaracao
 *        no canto de um bloco, a assinatura de uma funcao.
 *
 * A FONTE E A DO PLACAR (Battle::FonteDoPlacar): carregar uma por etiqueta
 * custaria vinte e nove tamanhos de TTF cada. A batalha vive mais que os
 * atores dela, e o texto so usa a fonte quando muda.
 */
class EtiquetaDeCodigo : public Actor {
public:
    EtiquetaDeCodigo(Scene* cena, const std::string& texto, int largura, int altura, int ordem);

    void Pintar(SDL_Color moldura, SDL_Color fundo, SDL_Color tinta);
    void Escrever(const std::string& texto);
    void Mostrar(bool mostrar);

    static constexpr int kTamanhoDaLetra = 20;

private:
    DrawCaixaComponent* mCaixa = nullptr;
    DrawTextComponent* mTexto = nullptr;
};
