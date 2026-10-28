//
// O chefe do TCC - Trabalho de Conclusao de Curso.
//

#pragma once

#include <array>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../Boss.h"
#include "../../../Attacks/Cronograma.h"
#include "../../../Attacks/Objetivo.h"
#include "../../../Attacks/Tcc.h"

class BossProjectile;
class DrawLinhasComponent;
class Font;
class Player;

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
 * E ELE E BLINDADO (Objetivo.h): o TCC ainda nao esta pronto. Em cada etapa o
 * aluno cumpre objetivos que abrem uma janela em que o Hugo toma dano, de duas
 * maneiras que se alternam: DERRUBAR os alvos - as assinaturas, os bugs, a
 * resposta certa da banca, as correcoes - ou ANDAR - a reuniao com o
 * orientador, as referencias, a arguicao (raspar nas perguntas), a entrega da
 * versao final. Aberto, as paginas da monografia escorrem embaixo dele. As
 * etapas duram 30 s (Fase::kDuracaoDoTcc): a batalha maior do jogo.
 *
 * O ESCUDO SE LE SEM O PLACAR (DesenharEscudo): blindado, ele fica frio e
 * escuro, com o cadeado e as placas, e um fio ate cada coisa que falta;
 * aberto, a mira dourada e o arco do tempo.
 *
 * O BALANCEAMENTO DO QUE VAI POR TEMPO VIVE EM DADOS (Assets/Attacks/hugo/);
 * aqui mora o cronograma, os padroes, a banca e o escudo.
 */
class Hugo : public Boss {
public:
    explicit Hugo(Scene* scene);
    ~Hugo() override;

    void OnUpdate(float deltaTime) override;

    [[nodiscard]] std::optional<Vector2> AlvoDoMovimento() const override { return mAlvo; }

    // ---- O ESCUDO (Objetivo.h) ----------------------------------------------

    /// Blindado, o tiro so carrega o poder do orientador: a nota so sobe com a
    /// janela aberta.
    [[nodiscard]] bool AceitaDano() const override { return !mEscudo; }
    bool QuebrarEscudo() override;
    void OnDerrubou(BossProjectile* qual) override;
    void AoTerminarABatalha() override;

    [[nodiscard]] bool Blindado() const { return mEscudo; }

    /// Onde esta o alvo que falta derrubar - na defesa, a resposta certa. Vazio
    /// com a janela aberta ou sem alvo em campo. Serve a quem quer mostrar o
    /// caminho (e ao piloto que mede a batalha).
    [[nodiscard]] std::optional<Vector2> ObjetivoAtual() const;

    /// Onde o aluno tem de IR, num objetivo de andar (Objetivo::Tipo): a
    /// reuniao, o livro mais perto, a versao final ou a secretaria. Vazio na
    /// arguicao (ali o caminho sao os tiros) e nos objetivos de derrubar.
    [[nodiscard]] std::optional<Vector2> LugarDeIr() const;

    /// Os alvos do objetivo que o tiro derruba e que contam (os certos), para
    /// os agentes do Julio: so com o escudo de pe.
    [[nodiscard]] std::vector<Vector2> AlvosDestrutiveis() const override;
    [[nodiscard]] Objetivo::Tipo TipoDoObjetivo() const { return mTipo; }

    /// O TIRO NO ESCUDO risca uma faisca onde bateu: e o que diz, sem texto,
    /// que ele nao entrou.
    void OnCollision(Actor* other) override;

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

    // ---- O ESCUDO E O OBJETIVO DA ETAPA ------------------------------------------

    void AtualizarObjetivo(float deltaTime);
    /// CONGELADO (o HLT do Ricardo): o relogio do Hugo continua - e o que diz
    /// a idade de cada tiro dele -, mas tudo o que tem hora marcada espera.
    void Adiar(float deltaTime);
    void NovoObjetivo();
    void AbrirJanela();
    void MatarAlvos();
    void DesenharEscudo();
    /// O placar: o texto da vez (mPrincipal) e, embaixo, o objetivo; na defesa,
    /// a pergunta com as respostas.
    void RefazerPlacar();

    struct AlvoEmCampo {
        BossProjectile* p = nullptr;
        float nasceu = 0.0f;
        Objetivo::Alvo alvo;
        Vector2 paragem;          ///< o bug anda ate aqui
        int k = 0;
        bool caiu = false;
    };
    Objetivo::Etapa mEtapa = Objetivo::Etapa::Nenhuma;
    bool mEscudo = false;
    float mJanelaAte = 0.0f;
    float mProximaPaginaSolta = 0.0f;
    float mProximoObjetivo = 0.0f;
    std::vector<AlvoEmCampo> mAlvos;
    Variacao::Sorteio mSorteioDoObjetivo{0u};
    Objetivo::Questao mQuestao;
    Golpe::Materia mQuemPergunta = Golpe::Materia::Generico;
    int mQuestoes = 0;
    std::string mPrincipal;
    std::string mPlacarFeito;
    DrawLinhasComponent* mEscudoLinhas = nullptr;
    DrawLinhasComponent* mCadeado = nullptr;

    // ---- AS OUTRAS MANEIRAS DE ABRIR (Objetivo::Tipo) -----------------------------

    Objetivo::Tipo mTipo = Objetivo::Tipo::Derrubar;
    int mVezDoObjetivo = 0;      ///< quantos objetivos ja houve nesta etapa
    int mPrimeiraManeira = 0;    ///< sorteada a cada etapa
    int mPlacas = 1;             ///< as placas do escudo deste objetivo
    bool mObjetivoEmCurso = false;   ///< ha um objetivo de pe (a reuniao nao tem alvo)
    float mProgresso = 0.0f;     ///< 0 a 1: quantas placas ja cairam

    /// Os alunos em campo: os objetivos de andar valem para qualquer um.
    [[nodiscard]] std::vector<Player*> Alunos() const;
    void AtualizarAndar(float deltaTime);
    void EncerrarAndar();
    void Pagar(float bruto);

    Vector2 mReuniao;            ///< o centro da reuniao
    float mNaReuniao = 0.0f;     ///< os segundos ja passados dentro dela
    bool mAlguemNaReuniao = false;

    struct Raspado {
        BossProjectile* p = nullptr;
        float nasceu = 0.0f;
    };
    std::vector<Raspado> mRaspados;   ///< os tiros que ja contaram
    int mRaspou = 0;

    Vector2 mSecretaria;
    Player* mCarregando = nullptr;    ///< quem esta com a versao final

    // ---- O DESENHO DO ESCUDO, SEM TEXTO ------------------------------------------

    struct Faisca {
        Vector2 onde;
        float quando = 0.0f;
        bool dourada = false;    ///< a raspada, e nao o tiro no escudo
    };
    std::vector<Faisca> mFaiscas;
    float mAbriuEm = -10.0f;      ///< para o estouro das placas
    float mFechouEm = -10.0f;     ///< para o escudo se refazendo
    float mApanhouEm = -10.0f;    ///< o clarao do acerto, aberto
    int mPlacasAoAbrir = 1;
    void Tingir();
    void DesenharFios();
    void DesenharAndar();

    /// A RESPOSTA ESCRITA embaixo de cada balao da defesa: no placar nao cabem
    /// tres respostas e a pergunta.
    std::unique_ptr<Font> mFonte;
    std::array<Actor*, 3> mRotulos{};
    void Rotular();
    void ApagarRotulos();

    // ---- FINAL: AS CORRECOES -------------------------------------------------------

    void AtualizarCorrecoes(float agora);
    float mProximaCorrecao = 0.0f;
    int mCorrecoes = 0;
};
