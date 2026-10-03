//
// Created by nslop on 04/06/2024.
//

#pragma once

#include "DrawComponent.h"
#include <string>

class DrawTextComponent : public DrawComponent {

public:

    DrawTextComponent(class Actor* owner, const std::string &text, class Font* font,
                        int width, int height, int fontSize = 24, int drawOrder = 100);
    ~DrawTextComponent();

    void SetText(const std::string &text);

    /**
     * @brief Desenhar no tamanho NATURAL da textura, em vez de esticar na caixa.
     *
     * O desenho sempre esticou o texto para preencher a largura e a altura
     * declaradas. Para um rotulo de tamanho fixo isso passa despercebido, mas num
     * campo que o usuario digita fica ruim: com um digito so, a textura de um
     * caractere e esticada para a caixa inteira e sai borrada e deformada - a
     * matricula "espichava" ate ficar completa.
     *
     * Com isto ligado, a caixa declarada vira um LIMITE: o texto e desenhado no
     * tamanho dele, centrado, e so encolhe (mantendo a proporcao) se nao couber.
     *
     * E opcional de proposito. Ligar para todo mundo mudaria a aparencia de todas
     * as telas que ja estao ajustadas ao comportamento antigo.
     */
    void SetAjustarAoTexto(const bool ajustar) { mAjustarAoTexto = ajustar; }

    /// @brief Em que largura o texto quebra linha. O padrao de 500 e o que sempre
    /// valeu; quem precisa de uma linha so e mais larga, aumenta.
    ///
    /// REDESENHA na hora. Se so guardasse o valor, um rotulo criado ja com o texto
    /// final ficaria com a quebra antiga para sempre, porque SetText nunca mais
    /// seria chamado nele - o ajuste funcionaria ou nao dependendo da ordem das
    /// chamadas, que e o tipo de armadilha que so aparece meses depois.
    void SetLarguraDeQuebra(unsigned largura);

    void Draw(SDL_Renderer* renderer) override;

protected:
    SDL_Texture* mTextSurface;
    class Font* mFont;

    int mSize;
    int mWidth;
    int mHeight;

    /// Ver SetAjustarAoTexto.
    bool mAjustarAoTexto = false;

    /// Ver SetLarguraDeQuebra. 500 era o valor fixo de antes.
    unsigned mLarguraDeQuebra = 500;

    /// O texto em exibicao, guardado so para poder redesenhar sem que quem chama
    /// precise repeti-lo.
    std::string mTexto;
};
