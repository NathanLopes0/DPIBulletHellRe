//
// A moldura do campo da batalha: as bordas onde o aluno nao vai.
//

#pragma once

#include <utility>
#include <vector>
#include <SDL_pixels.h>

#include "DrawComponent.h"
#include "../../Hud.h"

/**
 * @class DrawMolduraDoCampoComponent
 * @brief As BORDAS DO CAMPO, desenhadas onde o aluno de fato para.
 *
 * O campo nao tinha borda: o aluno parava a meia largura do lado da tela, e a
 * meia altura de cima da faixa da nota, sem nada que mostrasse por que. Agora:
 *
 *   dos lados, dois TRILHOS escuros, da largura que ele nao alcanca;
 *   embaixo, o PAINEL da faixa da nota comeca onde o pe dele para;
 *   em cima, uma linha tracejada discreta (o professor vive ali, e um painel
 *   esconderia os tiros que nascem dele);
 *   nos cantos, os ENCAIXES dos trilhos.
 *
 * E A MOLDURA RESPONDE: a parede acende perto do aluno quando ele encosta
 * (Hud::Encostado), e o campo inteiro pisca na cor de um acontecimento -
 * vermelho no acerto sofrido, ouro no Ponto Extra, lilas na troca de fase
 * (Acender).
 */
class DrawMolduraDoCampoComponent : public DrawComponent {
public:
    DrawMolduraDoCampoComponent(class Actor* owner, int drawOrder);

    void Update(float deltaTime) override;
    void Draw(SDL_Renderer* renderer) override;

    /// @brief O tamanho da tela e a area onde o desenho do aluno cabe.
    void Definir(float largura, float altura, const Hud::Caixa& area);

    /// @brief Os alunos deste quadro: o centro e o meio tamanho do desenho.
    void SetAlunos(std::vector<std::pair<Vector2, Vector2>> alunos) { mAlunos = std::move(alunos); }

    /// @brief ONDE O PROFESSOR ESTA, na horizontal: um brilho vermelho leve na
    /// borda de baixo, embaixo de cada corpo dele - quem esta olhando para o
    /// proprio pe sabe em que coluna ele esta sem levantar o olho.
    void SetProfessores(std::vector<float> xs) { mProfessores = std::move(xs); }

    /// @brief Um clarao de 'cor' nas bordas do campo, que some em 'duracao'.
    void Acender(SDL_Color cor, float duracao);

private:
    float mLargura = 0.0f;
    float mAltura = 0.0f;
    Hud::Caixa mArea;
    std::vector<std::pair<Vector2, Vector2>> mAlunos;
    std::vector<float> mProfessores;
    float mTempo = 0.0f;

    SDL_Color mCorDoClarao{255, 255, 255, 255};
    float mIdadeDoClarao = -1.0f;
    float mDuracaoDoClarao = 0.0f;

    void DesenharTrilho(SDL_Renderer* r, int x0, int x1, int y0, int y1, bool esquerdo) const;
    void DesenharPainel(SDL_Renderer* r) const;
    void DesenharEncostos(SDL_Renderer* r) const;
    void DesenharProfessores(SDL_Renderer* r) const;
    void DesenharClarao(SDL_Renderer* r) const;
};
