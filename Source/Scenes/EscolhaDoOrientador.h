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
#include "../Math.h"
#include "../Orientacao.h"
#include "../Painel.h"

class Actor;
class Font;
class DrawSpriteComponent;
class DrawCaixaComponent;

/**
 * @class EscolhaDoOrientador
 * @brief A tela entre a selecao de fases e o TCC: o aluno escolhe, entre os
 * professores das suas maiores notas, quem vai lutar do lado dele na banca.
 *
 * QUEM APARECE E QUEM VENCE E DE Orientacao, que e pura e testada: esta cena so
 * desenha os cartoes e passa o manche e os botoes para uma Orientacao::Votacao.
 * Em dupla cada aluno tem o seu cursor, da cor dele, e os dois votam; o
 * escolhido so existe quando os dois votaram.
 *
 * O ESCOLHIDO FICA ANUNCIADO por kAnuncio segundos antes de a batalha comecar,
 * e nesse tempo ainda da para desfazer o voto - o botao que confirma e o mesmo
 * que atira, e quem aperta duas vezes sem querer nao pode perder a escolha. So
 * depois o orientador vai para o Game (Game::DefinirOrientador), de onde a
 * batalha o le.
 *
 * O botao 2 desfaz o voto; sem voto para desfazer, o do jogador 1 volta para a
 * selecao de fases.
 */
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

    /// Quanto o escolhido fica na tela antes de a batalha comecar.
    static constexpr float kAnuncio = 1.8f;

    // ---- O cartao de cada professor ---------------------------------------
    static constexpr float kCartaoLargura = 290.0f;
    static constexpr float kCartaoAltura = 400.0f;
    static constexpr float kEntreCartoes = 370.0f;
    static constexpr float kCartaoY = 420.0f;

    /// O retrato do professor e o cartao de chefe dele (128 px), um pouco maior.
    static constexpr float kEscalaDoRetrato = 1.25f;

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

    /// A cor de cada aluno. Sozinho, o cursor e dourado: nao ha de quem ser.
    static inline const SDL_Color kCorSozinho{255, 214, 0, 255};
    static inline const std::array<SDL_Color, kMaxAlunos> kCorDoAluno{{
        {110, 190, 255, 255},
        {255, 160, 80, 255},
    }};

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

    std::array<Actor*, kMaxAlunos> mCursores{};    ///< a moldura de cada aluno
    std::array<Actor*, kMaxAlunos> mSelos{};       ///< "J1 VOTOU", sobre o cartao
    Actor* mEstado = nullptr;

    /// O escolhido que esta sendo anunciado, e ha quanto tempo.
    int mEscolhido = -1;
    float mAnunciando = 0.0f;
    bool mMudou = true;
    bool mSaindo = false;

    /// A borda de cada entrada, por aluno. Comecam em TRUE pelo motivo de
    /// StageSelect::mEntrarAnterior: chega-se aqui com o botao 1 apertado, e
    /// ele nao pode virar um voto que ninguem deu.
    struct Bordas {
        bool esquerda = true;
        bool direita = true;
        bool votar = true;
        bool desfazer = true;
    };
    std::array<Bordas, kMaxAlunos> mBordas{};
};
