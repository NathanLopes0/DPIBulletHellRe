//
// A moldura do campo: o fundo quadriculado, os trilhos dos lados e o "prof" embaixo de cada corpo.
//

#pragma once

#include <vector>

#include <SDL.h>

#include "../../Interface/Hud.h"

class Actor;
class Boss;
class DrawMolduraDoCampoComponent;
class Font;
class Player;
class Scene;

/// O fundo do campo (FundoDoCampo.h), que nao e preto liso, e a moldura em volta
/// (DrawMolduraDoCampoComponent): os trilhos que mostram onde estao os alunos e os professores.
/// Embaixo do brilho de cada corpo do professor, um "prof".
class MolduraDoCampo {
public:
    /// A area do aluno sai do desenho do primeiro de 'jogadores', o mesmo tamanho que o
    /// Player::PrenderNoCampo usa.
    MolduraDoCampo(Scene& cena, const SDL_FRect& campo, int larguraDaTela, int alturaDaTela,
                   const std::vector<Player*>& jogadores);

    /// Um quadro: onde estao os alunos e os corpos do professor. 'fonte': a do "prof" (a do placar).
    void Atualizar(const std::vector<Player*>& jogadores, const std::vector<Boss*>& corpos, Font* fonte);

    /// O desenho da moldura, que tambem acende nos claroes (ClaroesDaMoldura).
    [[nodiscard]] DrawMolduraDoCampoComponent& Desenho() const { return *mDesenho; }
    /// Ate onde o desenho do aluno chega.
    [[nodiscard]] const Hud::Caixa& AreaDoAluno() const { return mAreaDoAluno; }

private:
    Scene& mCena;
    DrawMolduraDoCampoComponent* mDesenho = nullptr;
    Hud::Caixa mAreaDoAluno;
    std::vector<Actor*> mRotulos;

    /// A moldura e o fundo, atras de tudo - ate do quadro (60).
    static constexpr int kOrdemDaMoldura = 40;
    static constexpr int kOrdemDoFundo = 10;
};
