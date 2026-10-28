//
// O relogio da fase: a barrinha de tempo que anda colada no professor.
//

#pragma once

#include "DrawComponent.h"

/**
 * @class DrawRelogioDaFaseComponent
 * @brief O tempo que falta para a fase acabar, numa capsula em pe ao lado do
 * professor.
 *
 * CONTINUA PEQUENA E COLADA NO CHEFE - e o que o olho alcanca sem sair do
 * combate. O que mudou e o que ela diz:
 *
 *   a capsula tem moldura, fundo e volume, e esvazia de cima para baixo;
 *   a cor anda do verde ao ambar e ao vermelho conforme o tempo acaba
 *   (Hud::CorDoRelogio);
 *   um DENTE a cada 5 s, para ler "quanto falta" sem contar pixels;
 *   nos ultimos 3 s ela PULSA, com um halo vermelho;
 *   na troca de fase ela se ENCHE de volta num clarao.
 *
 * A posicao do dono e o canto de cima a esquerda, como na ProgressBarComponent
 * vertical que ela substitui.
 */
class DrawRelogioDaFaseComponent : public DrawComponent {
public:
    DrawRelogioDaFaseComponent(class Actor* owner, int largura, int altura, int drawOrder = 200);

    void Update(float deltaTime) override;
    void Draw(SDL_Renderer* renderer) override;

    /// @brief O tempo que falta e o total da fase, em segundos.
    void SetTempo(float restante, float total);

    /// A fase trocou: a capsula se enche de volta, com um clarao.
    void Recomecar();

    [[nodiscard]] bool Urgente() const;

private:
    int mLargura;
    int mAltura;
    float mRestante = 1.0f;
    float mTotal = 1.0f;
    float mTempo = 0.0f;
    float mDesdeORecomeco = 10.0f;
};
