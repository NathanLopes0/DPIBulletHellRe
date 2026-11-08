//
// O desenho da barra da nota.
//

#pragma once

#include <SDL_pixels.h>

#include "DrawComponent.h"
#include "../../Interface/BarraDaNota.h"

/// A barra da nota na faixa de baixo da batalha.
class DrawBarraDaNotaComponent : public DrawComponent {
public:
    DrawBarraDaNotaComponent(class Actor* owner, int largura, int altura, int drawOrder = 200);

    void Update(float deltaTime) override;
    void Draw(SDL_Renderer* renderer) override;

    /// A barra parada em 'nota', sem animacao: o comeco da batalha.
    void Comecar(float nota);

    /// A nota deste quadro e a cor dela (Battle escolhe a cor da faixa). 'perigo' acende a moldura
    /// vermelha; 'cheia', a dourada.
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
