//
// A tela "Como Jogar", do menu principal.
//

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Scene.h"
#include "../Entrada/Borda.h"

class Font;
class Actor;
class DrawTextComponent;

/// O manual, uma pagina por vez: o objetivo, os controles, a nota, o que se destroi, a dupla e o
/// curso.
class TelaComoJogar : public Scene {
public:
    explicit TelaComoJogar(Game* game);
    ~TelaComoJogar() override = default;

    void Load() override;
    void OnProcessInput(const Uint8* keyState) override;

private:
    /// Cria um texto centrado no ponto, no tamanho natural dele.
    DrawTextComponent* Texto(const std::string& conteudo, float x, float y, int tamanho, int larguraMaxima);

    /// Escreve a pagina atual: titulo, linhas, indicador, setas e rodape.
    void Mostrar();

    std::unique_ptr<Font> mFonte;

    int mPagina = 0;

    DrawTextComponent* mTitulo = nullptr;
    std::vector<DrawTextComponent*> mLinhas;
    DrawTextComponent* mIndicador = nullptr;
    DrawTextComponent* mSetaAnterior = nullptr;
    DrawTextComponent* mSetaProxima = nullptr;
    DrawTextComponent* mRodape = nullptr;

    /// As bordas, para uma acao por toque.
    Borda mBordaVoltar{true};
    Borda mBordaAvancar{true};
    Borda mBordaEsquerda{true};
    Borda mBordaDireita{true};
};
