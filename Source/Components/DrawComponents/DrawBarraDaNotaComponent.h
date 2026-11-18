//
// O desenho da barra da nota.
//

#pragma once

#include <SDL_pixels.h>
#include <SDL_render.h>

#include "DrawComponent.h"
#include "../../Interface/BarraDaNota.h"

/// A barra da nota na faixa de baixo da batalha.
class DrawBarraDaNotaComponent : public DrawComponent {
public:
    DrawBarraDaNotaComponent(class Actor* owner, int largura, int altura, int drawOrder = 200);
    ~DrawBarraDaNotaComponent() override;

    void Update(float deltaTime) override;
    void Draw(SDL_Renderer* renderer) override;

    /// A barra parada em 'nota', sem animacao: o comeco da batalha.
    void Comecar(float nota);

    /// A nota deste quadro e a cor dela (Battle escolhe a cor da faixa). 'perigo' acende a moldura
    /// vermelha; 'cheia', a dourada.
    void SetNota(float nota, SDL_Color cor, bool perigo, bool cheia);
    /// A cor do cheio agora (a da nota): a dos cacos, quando a barra se quebra (FaixaDaNota::Quebrar).
    [[nodiscard]] SDL_Color GetCor() const { return mCor; }

    /// A barra fora do rodape (o Lucas Ferreira a usa de golpe): 'largura' de comprimento, girada
    /// 'angulo' graus (horario, como o SDL) em volta do proprio centro, e espelhada - enche do outro
    /// fim. O dono continua no canto de cima da barra deitada. Girada, ela e desenhada numa textura
    /// que gira; sem textura de alvo no renderizador, sai deitada.
    void DefinirPose(int largura, float angulo, bool espelhada);
    /// O desenho de sempre, no tamanho do rodape.
    void TirarPose();
    /// A nota que a barra mostra agora (a que persegue a de verdade): e o que se ve - e o que machuca.
    [[nodiscard]] float Mostrada() const { return mEstado.mostrada; }

private:
    void Desenhar(SDL_Renderer* renderer, int x, int y);

    int mLargura;
    int mAltura;
    int mLarguraDoRodape;
    bool mPosada = false;
    float mAngulo = 0.0f;
    bool mEspelhada = false;
    SDL_Texture* mTextura = nullptr;
    int mLarguraDaTextura = 0;
    int mAlturaDaTextura = 0;
    BarraDaNota::Estado mEstado;
    float mNota = 0.0f;
    SDL_Color mCor{200, 150, 0, 255};
    bool mPerigo = false;
    bool mCheia = false;
    float mTempo = 0.0f;
};
