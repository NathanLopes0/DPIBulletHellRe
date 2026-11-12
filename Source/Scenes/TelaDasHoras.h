//
// A tela das horas complementares: as atividades do curso, e as que o aluno ja cumpriu.
//

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Scene.h"
#include "../Entrada/Borda.h"
#include "../Motor/Math.h"

class Font;
class Actor;

/// O quadro das horas complementares de quem esta jogando (Horas.h): cada atividade, o que ela
/// pede, quanto vale e quantas vezes ja foi cumprida, e em cima as horas contra a meta. Abre do
/// boletim (Botao 1) e volta para ele. Na dupla, o manche troca de aluno.
class TelaDasHoras : public Scene {
public:
    explicit TelaDasHoras(Game* game);
    ~TelaDasHoras() override = default;

    void Load() override;
    void OnProcessInput(const Uint8* keyState) override;

private:
    Actor* Texto(float x, float y, int tamanho, int largura, bool aEsquerda = false);
    static void Escrever(Actor* ator, const std::string& texto, const Vector3& cor);
    void Redesenhar();

    std::unique_ptr<Font> mFonte;
    size_t mAluno = 0;

    Actor* mAlunoAtor = nullptr;
    Actor* mTotalAtor = nullptr;
    struct LinhaDeTela {
        Actor* titulo = nullptr;
        Actor* descricao = nullptr;
        Actor* horas = nullptr;
        Actor* cumpridas = nullptr;
    };
    std::vector<LinhaDeTela> mLinhas;
    Actor* mRodapeAtor = nullptr;

    Borda mBordaVoltar{true};
    Borda mBordaEsquerda{true};
    Borda mBordaDireita{true};
};
