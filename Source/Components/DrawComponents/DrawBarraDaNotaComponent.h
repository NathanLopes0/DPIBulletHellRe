//
// O desenho da barra da nota.
//

#pragma once

#include <SDL_pixels.h>

#include "DrawComponent.h"
#include "../../UI/BarraDaNota.h"

/**
 * @class DrawBarraDaNotaComponent
 * @brief A barra da nota na faixa de baixo da batalha.
 *
 * ERA UM RETANGULO CHAPADO com um risco claro na ponta: lia-se o numero, e so.
 * Agora a barra conta o que aconteceu sem o numero:
 *
 *   o preenchimento tem VOLUME (claro em cima, escuro embaixo) e as pontas
 *   arredondadas, e um reflexo atravessa ele de tempos em tempos;
 *   as MARCAS do 40 (o exame, ambar) e do 60 (a aprovacao, verde) ficam na
 *   barra, com um dente em cima e embaixo - da para ver quanto falta;
 *   o ACERTO deixa um rastro claro do que foi perdido, que encolhe, e a barra
 *   sacode; a nota que SOBE da um clarao (BarraDaNota.h);
 *   abaixo do 40 a moldura pulsa em vermelho; no 100, brilha em ouro.
 *
 * A posicao do dono e o canto de cima a esquerda da barra, como na
 * ProgressBarComponent que ela substitui.
 */
class DrawBarraDaNotaComponent : public DrawComponent {
public:
    DrawBarraDaNotaComponent(class Actor* owner, int largura, int altura, int drawOrder = 200);

    void Update(float deltaTime) override;
    void Draw(SDL_Renderer* renderer) override;

    /// A barra parada em 'nota', sem animacao: o comeco da batalha.
    void Comecar(float nota);

    /// @brief A nota deste quadro e a cor dela (Battle escolhe a cor da faixa).
    /// 'perigo' acende a moldura vermelha; 'cheia', a dourada.
    void SetNota(float nota, SDL_Color cor, bool perigo, bool cheia);

private:
    int mLargura;
    int mAltura;
    BarraDaNota::Estado mEstado;
    float mNota = 0.0f;
    SDL_Color mCor{200, 150, 0, 255};
    bool mPerigo = false;
    bool mCheia = false;
    float mTempo = 0.0f;
};
