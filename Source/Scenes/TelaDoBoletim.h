//
// A tela do boletim do semestre: as materias do aluno, periodo a periodo.
//

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Scene.h"
#include "../Entrada/Borda.h"
#include "../Aluno/Boletim.h"
#include "../Aluno/Exportacao.h"
#include "../Motor/Math.h"

class Font;
class Actor;

/// O boletim de quem esta jogando (Boletim.h): a melhor nota, a situacao e as tentativas de cada
/// materia, periodo a periodo, e embaixo o CR, as aprovadas e a posicao no curso. Abre da selecao
/// de materias (Botao 3); dali, o Botao 3 leva ao ranking da materia em foco. Na dupla, o manche
/// troca de aluno.
class TelaDoBoletim : public Scene {
public:
    explicit TelaDoBoletim(Game* game);
    ~TelaDoBoletim() override = default;

    void Load() override;
    void OnProcessInput(const Uint8* keyState) override;

private:
    Actor* Texto(const std::string& conteudo, float x, float y, int tamanho, int larguraMaxima,
                 bool aEsquerda = false);
    static void Escrever(Actor* ator, const std::string& texto, const Vector3& cor);

    /// Reescreve a tela com o boletim do aluno 'mAluno'.
    void Redesenhar();

    std::unique_ptr<Font> mFonte;
    /// A turma, lida uma vez: a posicao no curso sai dela.
    std::vector<Exportacao::FichaDeAluno> mFichas;
    /// De quem e o boletim na tela: a posicao no painel.
    size_t mAluno = 0;

    Actor* mAlunoAtor = nullptr;
    /// Uma linha da tabela por linha de tela: o periodo, ou uma materia nas cinco colunas.
    struct LinhaDeTela {
        Actor* materia = nullptr;
        Actor* professor = nullptr;
        Actor* nota = nullptr;
        Actor* situacao = nullptr;
        Actor* tentativas = nullptr;
    };
    std::vector<LinhaDeTela> mLinhas;
    Actor* mResumoAtor = nullptr;
    Actor* mPosicaoAtor = nullptr;
    Actor* mRodapeAtor = nullptr;

    Borda mBordaVoltar{true};
    Borda mBordaRanking{true};
    Borda mBordaEsquerda{true};
    Borda mBordaDireita{true};

    /// Quantas linhas a tabela reserva: as materias e um titulo por periodo, do curso de hoje com
    /// folga.
    static constexpr size_t kLinhasDaTabela = 20;
};
