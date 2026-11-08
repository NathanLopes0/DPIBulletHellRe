//
// A tela "Como Jogar", do menu principal.
//

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Scene.h"

class Font;
class Actor;
class DrawTextComponent;

/**
 * @class TelaComoJogar
 * @brief O manual, uma pagina por vez: o objetivo, os controles, a nota, o que
 * se destroi, a dupla e o curso.
 *
 * O TEXTO NAO E DAQUI: vem de Assets/como-jogar.json (Manual.h). Esta cena so
 * desenha a pagina em que o leitor esta e vira as paginas.
 *
 * O manche para os lados vira a pagina; o botao 1 vai para a seguinte e, na
 * ultima, volta ao menu; o botao 2 (ou o ESC) volta ao menu de qualquer
 * pagina. No gabinete, as linhas que falam de tecla nao aparecem.
 */
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

    /// As bordas, para uma acao por toque. Comecam em TRUE pela mesma regra
    /// das outras cenas: quem entra aqui com o botao 1 do menu ainda apertado
    /// nao pula a primeira pagina sem querer.
    bool mVoltarAnterior = true;
    bool mAvancarAnterior = true;
    bool mEsquerdaAnterior = true;
    bool mDireitaAnterior = true;
};
