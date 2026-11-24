//
// O relogio da fase: a ampulheta que anda colada no professor.
//

#pragma once

#include "DrawComponent.h"

/// O tempo que falta para a fase acabar, numa ampulheta em pe ao lado do professor: a areia cai do
/// bulbo de cima para o de baixo, e na troca de fase ela vira meia volta.
class DrawRelogioDaFaseComponent : public DrawComponent {
public:
    DrawRelogioDaFaseComponent(class Actor* owner, int largura, int altura, int drawOrder = 200);

    void Update(float deltaTime) override;
    void Draw(SDL_Renderer* renderer) override;

    /// O tempo que falta e o total da fase, em segundos.
    void SetTempo(float restante, float total);

    /// A fase trocou: a ampulheta vira, com um clarao.
    void Recomecar();

private:
    int mLargura;
    int mAltura;
    float mRestante = 1.0f;
    float mTotal = 1.0f;
    float mTempo = 0.0f;
    float mDesdeORecomeco = 10.0f;
};
