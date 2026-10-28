//
// O quadro da sala: o placar da batalha, de giz.
//

#pragma once

#include "DrawComponent.h"

/**
 * @class DrawQuadroComponent
 * @brief O QUADRO-NEGRO do canto da batalha, onde o professor escreve o que
 * esta fazendo.
 *
 * Era uma caixa verde com um fio cinza. Agora e um quadro: a moldura de
 * madeira, a lousa verde com o po de giz, a canaleta embaixo com um toco de
 * giz - e, quando o texto troca, o APAGADOR passa (Apagar): o aluno ve que ha
 * coisa nova escrita, sem precisar comparar com o que estava ali.
 *
 * A posicao do dono e o CENTRO do quadro, como na DrawCaixaComponent.
 */
class DrawQuadroComponent : public DrawComponent {
public:
    DrawQuadroComponent(class Actor* owner, int largura, int altura, int drawOrder);

    void Update(float deltaTime) override;
    void Draw(SDL_Renderer* renderer) override;

    /// O texto trocou: o apagador passa.
    void Apagar() { mDesdeOApagador = 0.0f; }

private:
    int mLargura;
    int mAltura;
    float mDesdeOApagador = 10.0f;
};
