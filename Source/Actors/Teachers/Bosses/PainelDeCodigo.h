//
// Paineis e linhas de codigo desenhados no campo.
//

#pragma once

#include <string>

#include <SDL_pixels.h>

#include "../../Actor.h"

class DrawCaixaComponent;
class DrawTextComponent;

/// Um retangulo com fundo e moldura, centrado no ator: um bloco do programa escrito no campo.
class PainelDeCodigo : public Actor {
public:
    PainelDeCodigo(Scene* cena, int largura, int altura, int ordem);

    void Pintar(SDL_Color moldura, SDL_Color fundo);
    void Mostrar(bool mostrar);

private:
    DrawCaixaComponent* mCaixa = nullptr;
};

/// Uma linha de codigo numa caixinha, na fonte do placar: a declaracao no canto de um bloco, a
/// assinatura de uma funcao.
class EtiquetaDeCodigo : public Actor {
public:
    EtiquetaDeCodigo(Scene* cena, const std::string& texto, int largura, int altura, int ordem);

    void Pintar(SDL_Color moldura, SDL_Color fundo, SDL_Color tinta);
    void Escrever(const std::string& texto);
    void Mostrar(bool mostrar);

    /// Onde o texto quebra de linha, em pixels. O padrao e uma linha so: uma etiqueta de duas
    /// linhas precisa quebrar na largura dela, ou o texto encolhe para caber.
    void QuebrarEm(int largura);

    static constexpr int kTamanhoDaLetra = 20;

private:
    DrawCaixaComponent* mCaixa = nullptr;
    DrawTextComponent* mTexto = nullptr;
};
