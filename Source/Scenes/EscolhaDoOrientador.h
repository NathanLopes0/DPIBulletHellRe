//
// Antes do TCC: quem vai lutar do lado do aluno.
//

#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>

#include <SDL_pixels.h>

#include "Scene.h"
#include "../Motor/Math.h"
#include "../Curso/Orientacao.h"
#include "../Entrada/Painel.h"
#include "../Regras/Poder.h"

class Actor;
class Font;
class DrawSpriteComponent;
class DrawCaixaComponent;
class DrawLinhasComponent;

/// A tela entre a selecao de fases e o TCC: o aluno escolhe, entre os professores das suas maiores
/// notas, quem vai lutar do lado dele na banca. Escolher o orientador e escolher um poder, e o
/// cartao de cada um mostra o dele funcionando: um campo em miniatura, animado (Poder::Previa), com
/// a legenda de cada passo.
class EscolhaDoOrientador : public Scene {
public:
    explicit EscolhaDoOrientador(Game* game);
    ~EscolhaDoOrientador() override = default;

    void Load() override;

protected:
    void OnProcessInput(const Uint8* keyState) override;
    void OnUpdate(float deltaTime) override;

private:
    /// Quantos alunos o painel tem.
    static constexpr int kMaxAlunos = 2;

    static constexpr float kAnuncio = 1.8f;

    // ---- O cartao de cada professor ----
    static constexpr float kCartaoLargura = 290.0f;
    static constexpr float kCartaoAltura = 500.0f;
    static constexpr float kEntreCartoes = 370.0f;
    static constexpr float kCartaoY = 408.0f;

    /// O retrato do professor e o cartao de chefe dele (128 px).
    static constexpr float kEscalaDoRetrato = 1.0f;

    /// O que o cartao mostra, de cima para baixo, em px a partir do topo dele.
    static constexpr float kRetratoEm = 70.0f;
    static constexpr float kProfessorEm = 152.0f;
    static constexpr float kMateriaEm = 180.0f;
    static constexpr float kNomeDaMateriaEm = 218.0f;
    static constexpr float kNotaEm = 256.0f;
    static constexpr float kPoderEm = 290.0f;
    static constexpr float kPreviaEm = 368.0f;      ///< o meio do campo em miniatura
    static constexpr float kLegendaEm = 440.0f;
    static constexpr float kEfeitoEm = 474.0f;

    /// O campo em miniatura da previa: duas vezes mais largo que alto, como Poder::Previa desenha.
    static constexpr float kPreviaLargura = 248.0f;
    static constexpr float kPreviaAltura = 112.0f;
    static inline const SDL_Color kMolduraDaPrevia{70, 86, 130, 230};
    static inline const SDL_Color kFundoDaPrevia{6, 8, 18, 235};

    /// O selo do voto fica entre o cabecalho e o cartao.
    static constexpr float kSeloAcima = 38.0f;

    /// O curriculo de orientacoes (Progresso::Orientacoes), no canto do cartao: quem ja orientou um
    /// TCC aprovado de quem esta jogando, e quem ainda e novo - o que falta para a mencao dele.
    static constexpr float kCurriculoLargura = 104.0f;
    static constexpr float kCurriculoAltura = 22.0f;
    static inline const SDL_Color kCorDeQuemOrientou{186, 158, 250, 255};
    static inline const SDL_Color kCorDoNovo{255, 214, 72, 255};
    void CriarCurriculo(int indice, const Vector2& topo);

    /// O cursor de cada aluno e uma moldura em volta do cartao. A do J2 e mais
    /// larga que a do J1, para as duas aparecerem quando estao no mesmo cartao.
    static constexpr float kFolgaDoCursor = 12.0f;
    static constexpr float kFolgaEntreCursores = 14.0f;
    static constexpr int kEspessuraDoCursor = 4;

    /// A ordem de desenho: fundo do cartao, retrato e texto, e o cursor por cima.
    static constexpr int kOrdemDoCartao = 70;
    static constexpr int kOrdemDoRetrato = 90;
    static constexpr int kOrdemDoCursor = 120;

    static inline const SDL_Color kMolduraDoCartao{96, 118, 170, 220};
    static inline const SDL_Color kFundoDoCartao{10, 14, 30, 215};
    static inline const SDL_Color kSemFundo{0, 0, 0, 0};

    /// A cor de cada aluno e Hud::CorDoAluno - a mesma do "?" da Duvida dele.
    /// Sozinho, o cursor e dourado: nao ha de quem ser.
    static inline const SDL_Color kCorSozinho{255, 214, 0, 255};

    /// O retrato de quem nao foi escolhido escurece enquanto o anuncio dura.
    static constexpr Uint8 kRetratoApagado = 90;

    [[nodiscard]] static Painel::Jogador DoAluno(int aluno);
    [[nodiscard]] static Vector3 ParaVector3(const SDL_Color& cor);
    [[nodiscard]] SDL_Color CorDoAluno(int aluno) const;
    [[nodiscard]] bool EmDupla() const { return mVotacao.Alunos() > 1; }

    /// Um texto centrado em 'onde', que encolhe para caber na largura.
    Actor* Texto(const std::string& conteudo, const Vector2& onde, int tamanho,
                 int largura, const Vector3& cor, int linhas = 1);
    static void Escrever(Actor* ator, const std::string& texto, const Vector3& cor);

    void CriarCabecalho();
    void CriarCartao(int indice);
    /// O poder do cartao 'indice': o nome, o campo em miniatura, a legenda e o que ele faz.
    void CriarPrevia(int indice, const Vector2& topo);
    /// Um quadro da previa de cada cartao.
    void DesenharPrevias();
    [[nodiscard]] static SDL_Color CorDaMarca(Poder::Marca marca);
    void CriarCursores();
    void CriarRodape();

    /// Redesenha cursores, selos e a linha de estado a partir da votacao.
    void Atualizar();
    [[nodiscard]] std::string FraseDoEstado(Vector3& cor) const;

    std::unique_ptr<Font> mFonte;

    std::vector<Orientacao::Candidato> mCandidatos;
    Orientacao::Votacao mVotacao{0, 1};

    std::vector<Vector2> mCentros;                 ///< o centro de cada cartao
    std::vector<DrawSpriteComponent*> mRetratos;   ///< observadores, um por cartao

    /// A previa do poder de cada cartao: o que e, onde se desenha, e a legenda que ja esta escrita.
    struct PreviaDoCartao {
        Poder::Tipo tipo = Poder::Tipo::Nenhum;
        Vector2 canto;                              ///< o canto de cima, a esquerda, do campo em miniatura
        DrawLinhasComponent* linhas = nullptr;
        Actor* legenda = nullptr;
        std::string escrita;
    };
    std::vector<PreviaDoCartao> mPrevias;
    float mTempoDaPrevia = 0.0f;

    std::array<Actor*, kMaxAlunos> mCursores{};    ///< a moldura de cada aluno
    std::array<Actor*, kMaxAlunos> mSelos{};       ///< "J1 VOTOU", sobre o cartao
    Actor* mEstado = nullptr;

    /// O escolhido que esta sendo anunciado, e ha quanto tempo.
    int mEscolhido = -1;
    float mAnunciando = 0.0f;
    bool mMudou = true;
    bool mSaindo = false;

    /// A borda de cada entrada, por aluno.
    struct Bordas {
        bool esquerda = true;
        bool direita = true;
        bool votar = true;
        bool desfazer = true;
    };
    std::array<Bordas, kMaxAlunos> mBordas{};
};
