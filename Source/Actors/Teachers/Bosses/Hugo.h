//
// O chefe do TCC - Trabalho de Conclusao de Curso.
//

#pragma once

#include <array>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "../Boss.h"
#include "../../../Attacks/Cronograma.h"
#include "../../../Attacks/Tcc.h"

class BossProjectile;
class DrawLinhasComponent;

/**
 * @class Hugo
 * @brief O professor do TCC, a ultima materia do curso - e a batalha mais
 * dificil do jogo.
 *
 * E A UNICA BATALHA QUE O ALUNO NAO LUTA SOZINHO: antes dela ele escolhe um
 * ORIENTADOR entre os professores das materias em que foi melhor
 * (Orientacao.h), e o orientador luta do lado dele com o golpe da materia
 * (Golpe.h). E O TCC E NA AREA DO ORIENTADOR: o Hugo usa a mesma materia
 * contra o aluno, e a banca e de outros dois professores (Tcc.h).
 *
 * AS FASES SAO AS ETAPAS DE UM TCC:
 *
 *   fase 1  a proposta        o cronograma: um grafico de Gantt no alto, e
 *                             cada barra cai quando o hoje passa do prazo
 *                             dela (Cronograma.h)
 *   fase 2  o desenvolvimento cada capitulo e um padrao da materia do
 *                             orientador, com os projeteis dela (Tcc::Padrao)
 *   fase 3  a defesa          a banca - dois outros professores, sentados nos
 *                             cantos - pergunta, cada um com a materia dele
 *   final   as correcoes      o tema e as perguntas da banca voltam
 *
 * O BALANCEAMENTO DO QUE VAI POR TEMPO VIVE EM DADOS (Assets/Attacks/hugo/);
 * aqui mora o cronograma, os padroes e a banca.
 */
class Hugo : public Boss {
public:
    explicit Hugo(Scene* scene);
    ~Hugo() override = default;

    void OnUpdate(float deltaTime) override;

    [[nodiscard]] std::optional<Vector2> AlvoDoMovimento() const override { return mAlvo; }

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;
    void OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                              const std::string& stateName) override;

private:
    // ---- A LUTA SORTEADA ----------------------------------------------------

    void SortearALuta();

    bool mLutaSorteada = false;
    Golpe::Materia mTema = Golpe::Materia::Generico;   ///< a materia do orientador
    std::string mOrientador;                           ///< o nome dele, para o placar
    std::array<Golpe::Materia, 2> mBanca{};
    Variacao::Sorteio mSorteioDoCronograma{0u};
    Variacao::Sorteio mSorteioDosPadroes{0u};
    Cronograma::Degrau mDegrauDoCronograma;
    Tcc::Degrau mDegrau;

    float mRelogio = 0.0f;     ///< o tempo da batalha, para a idade de cada tiro
    Vector2 mAlunoAntes;
    Vector2 mAndando;          ///< a velocidade do aluno, suavizada: a mira adianta
    bool mViuOAluno = false;
    /// Onde mirar o tiro que sai de 'de' a 'velocidade' (Tcc::Mira).
    [[nodiscard]] Vector2 MiraDe(const Vector2& de, float velocidade, float adiantamento) const;
    std::optional<Vector2> mAlvo;

    /// Dispara 'gatilho' de 'onde', no rumo 'rumo' (graus). Devolve o que nasceu.
    std::vector<BossProjectile*> DispararDe(const char* gatilho, Vector2 onde, float rumo);
    [[nodiscard]] Vector2 PosicaoDoAluno() const;
    [[nodiscard]] float Largura() const;
    [[nodiscard]] static bool Vivo(const BossProjectile* p, float idade);
    void MatarPosto(const std::string& posto);
    void Anunciar(const std::string& fase, const std::string& texto);

    bool mDisparandoDe = false;
    Vector2 mOndeDisparar;
    float mRumoDoDisparo = 90.0f;
    std::vector<BossProjectile*> mDisparados;

    DrawLinhasComponent* mLinhas = nullptr;
    void CriarLinhas();

    /// A fase em que ele esta agora, e o que fazer ao entrar e ao sair de uma.
    std::string mFase;
    void TrocarDeFase(const std::string& nova);
    /// O anuncio de cada fase como o arquivo o escreveu: o placar volta a ele
    /// quando a fase acaba.
    std::map<std::string, std::string> mAnunciosDoArquivo;

    // ---- OS PADROES DAS MATERIAS ---------------------------------------------
    //
    // Um padrao (Tcc::Padrao) sai de uma FONTE - o Hugo, ou um dos dois da
    // banca - mirado no aluno; cada tiro que espera, curva, volta, oscila ou
    // se divide e acompanhado (Golpe::Avancar).

    enum class Fonte { Hugo, Banca0, Banca1 };
    [[nodiscard]] Vector2 PosicaoDa(Fonte f) const;
    [[nodiscard]] float Adiantamento(Fonte f) const;

    void Lancar(Golpe::Materia m, Fonte f);
    void AtualizarPadroes(float deltaTime);
    void Soltar(Golpe::Materia m, Fonte f, const Golpe::Tiro& tiro, const Vector2& de, const Vector2& direcao);

    struct Execucao {
        Golpe::Materia materia = Golpe::Materia::Generico;
        Fonte fonte = Fonte::Hugo;
        Golpe::Rodada rodada;
        float inicio = 0.0f;
        size_t proximo = 0;
    };
    struct Guiado {
        BossProjectile* p = nullptr;
        float nasceu = 0.0f;
        Golpe::Materia materia = Golpe::Materia::Generico;
        Fonte fonte = Fonte::Hugo;
        Golpe::Voo voo;
    };
    std::vector<Execucao> mExecucoes;
    std::vector<Guiado> mGuiados;
    int mVezes = 0;            ///< quantos padroes ja sairam: o k de Tcc::Padrao

    // ---- FASE 1: O CRONOGRAMA ---------------------------------------------------

    void NovoCronograma();
    void AtualizarCronograma();
    void DesenharCronograma(float hoje);

    struct Barra {
        Cronograma::Tarefa tarefa;
        std::vector<BossProjectile*> paginas;
        float nasceu = 0.0f;
        bool caiu = false;
    };
    std::vector<Barra> mBarras;
    float mInicioDoCronograma = 0.0f;

    // ---- FASE 2: O DESENVOLVIMENTO ------------------------------------------------

    void AtualizarTema(float agora);
    float mProximoTema = 0.0f;
    int mCapitulo = 0;
    float mTempoVarrendo = 0.0f;
    float mXDoComeco = 0.0f;
    float mUltimoTema = -10.0f;
    int mRascunhos = 0;        ///< as paginas que ja escorreram neste capitulo

    // ---- FASE 3: A DEFESA ----------------------------------------------------------

    void AtualizarDefesa(float agora);
    void ChamarABanca();
    void DispensarABanca();
    std::array<Actor*, 2> mCadeiras{};
    float mProximaPergunta = 0.0f;
    int mPerguntas = 0;

    /// As anotacoes que escorrem dele na defesa e nas correcoes.
    void Anotar(float agora);
    float mProximaAnotacao = 0.0f;

    // ---- FINAL: AS CORRECOES -------------------------------------------------------

    void AtualizarCorrecoes(float agora);
    float mProximaCorrecao = 0.0f;
    int mCorrecoes = 0;
};
