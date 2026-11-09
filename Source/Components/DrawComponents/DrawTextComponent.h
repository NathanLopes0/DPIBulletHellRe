//
// Created by nslop on 04/06/2024.
//

#pragma once

#include "DrawComponent.h"
#include <string>
#include "../../Motor/Math.h"

class DrawTextComponent : public DrawComponent {

public:

    DrawTextComponent(class Actor* owner, const std::string &text, class Font* font,
                        int width, int height, int fontSize = 24, int drawOrder = 100);
    ~DrawTextComponent();

    void SetText(const std::string &text);

    /// Desenhar no tamanho natural da textura, em vez de esticar na caixa. O desenho sempre esticou
    /// o texto para preencher a largura e a altura declaradas.
    void SetAjustarAoTexto(const bool ajustar) { mAjustarAoTexto = ajustar; }

    /// Em que largura o texto quebra linha.
    void SetLarguraDeQuebra(unsigned largura);

    /// A cor do texto.
    void SetColor(const Vector3& cor);

    void Draw(SDL_Renderer* renderer) override;

protected:
    void Redesenhar();

    SDL_Texture* mTextSurface;
    class Font* mFont;

    int mSize;
    int mWidth;
    int mHeight;

    bool mAjustarAoTexto = false;

    unsigned mLarguraDeQuebra = 500;

    Vector3 mCor = Color::White;

    /// Os pixels da tela por pixel do jogo com que a textura foi desenhada (EscalaDaTela). Quando a
    /// tela muda (janela, tela cheia), o texto e redesenhado no tamanho novo.
    float mEscala = 1.0f;

    /// O texto em exibicao: e com ele que SetText sabe que nada mudou, e que a
    /// cor e a quebra redesenham sem que quem chama precise repeti-lo.
    std::string mTexto;
};
