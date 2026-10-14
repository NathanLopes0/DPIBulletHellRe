//
// Created by nslop on 04/06/2024.
//

#pragma once

#include "DrawComponent.h"
#include <string>
#include "../../Math.h"

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

    /// @brief A cor do texto. Branco e o padrao, que e o que sempre valeu.
    ///
    /// REDESENHA na hora, pela mesma razao de SetLarguraDeQuebra: a cor entra na
    /// textura no momento em que ela e criada, entao guardar o valor sem
    /// redesenhar faria a cor pegar ou nao conforme a ordem das chamadas.
    ///
    /// Chamar com a cor que ja esta nao faz nada. Isso importa: quem pinta a nota
    /// chama isto A CADA QUADRO, e refazer a textura 60 vezes por segundo para
    /// nada e justamente o tipo de desperdicio que nao aparece em teste nenhum.
    void SetColor(const Vector3& cor);

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

    /// Ver SetColor. Branco era o valor fixo de antes.
    Vector3 mCor = Color::White;

    /// O texto em exibicao, guardado so para poder redesenhar sem que quem chama
    /// precise repeti-lo.
    std::string mTexto;
};
