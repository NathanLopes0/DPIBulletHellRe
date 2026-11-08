//
// O chefe da INF 321 - Programacao Web.
//

#pragma once

#include <array>
#include <optional>
#include <string>
#include <vector>

#include "../Boss.h"
#include "../../../Attacks/Pagina.h"
#include "../../../Attacks/Elixir.h"
#include "../../../Attacks/Estilo.h"
#include "../../../Attacks/EventLoop.h"
#include "../../../Attacks/Flex.h"

class BossProjectile;
class DrawLinhasComponent;

/// O professor de Programacao Web (INF 321).
class Lucas : public Boss {
public:
    explicit Lucas(Scene* scene);
    ~Lucas() override = default;

    void OnUpdate(float deltaTime) override;

    [[nodiscard]] std::optional<Vector2> AlvoDoMovimento() const override { return mAlvo; }

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;
    void OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                              const std::string& stateName) override;

private:
    // ---- A luta sorteada ----

    void SortearALuta();

    bool mLutaSorteada = false;
    Variacao::Sorteio mSorteioDaPagina{0u};
    Variacao::Sorteio mSorteioDoEstilo{0u};
    Estilo::Degrau mDegrauDoEstilo;
    Pagina::Degrau mDegrauDaPagina;
    Elixir::Degrau mDegrauDoElixir;

    std::optional<Vector2> mAlvo;

    /// Dispara 'gatilho' de 'onde', no rumo 'rumo' - ou mirado no aluno, sem
    /// rumo. Devolve o que nasceu; com 'enfeite', o que nasce nao machuca.
    std::vector<BossProjectile*> DispararDe(const char* gatilho, Vector2 onde, std::optional<float> rumo,
                                            bool enfeite = false);

    bool mDisparandoDe = false;
    Vector2 mOndeDisparar;
    float mRumoDoDisparo = 90.0f;
    bool mEnfeiteAoNascer = false;
    std::vector<BossProjectile*> mDisparados;

    /// O tiro guardado ainda e o mesmo se a idade dele bate com a esperada: o
    /// pool o devolve com o relogio zerado.
    [[nodiscard]] static bool Vivo(const BossProjectile* p, float idade);

    /// O desenho da fase em curso: o layout da pagina, a arvore de supervisao...
    DrawLinhasComponent* mLinhas = nullptr;
    void CriarLinhas();

    // ---- A pagina da fase 1 ----

    void AtualizarPagina(float deltaTime);
    void RenderizarPagina(float deltaTime);
    void DesenharPagina();
    void RecarregarPagina();

    struct Renderizado {
        Pagina::Elemento e;
        float idade = 0.0f;
        int passos = 0;                       ///< linhas da tabela, letras do input
        bool feito = false;
        bool cancelado = false;               ///< o cursor derrubado: o clique nao veio
        BossProjectile* cursor = nullptr;
        std::vector<BossProjectile*> spinner;
    };

    bool mNaPagina = false;
    std::string mAnuncioDaPagina;
    std::vector<Pagina::Elemento> mPagina;
    std::vector<Renderizado> mRenderizados;
    size_t mProximoElemento = 0;
    float mProximoRender = 0.0f;
    bool mRecarregando = false;
    float mXDoProfessor = 0.0f;

    // ---- O devtools da fase 2 ----

    void AtualizarEstilo(float deltaTime);
    void RepovoarGrade(float agora);

    bool mNoEstilo = false;
    std::string mAnuncioDoEstilo;
    Estilo::Css mCss;
    Estilo::Regra mRegra;
    float mProximaRegra = 0.0f;
    float mInicioDaRegra = 0.0f;
    std::array<BossProjectile*, Estilo::kCartoes> mCartoes{};
    std::array<float, Estilo::kCartoes> mNascimentoDosCartoes{};
    bool mCaindo = false;                  ///< o position: fixed em curso
    bool mRepintou = false;                ///< o repaint desta regra ja saiu
    int mLetrasEscorridas = 0;             ///< a cascata desta regra
    float mXDoComecoDaVarredura = 0.0f;
    float mTempoVarrendo = 0.0f;           ///< o relogio da varredura, que para enquanto ele digita

    // ---- A arvore de supervisao da fase 3 ----

    void AtualizarSupervisao(float deltaTime);
    void IniciarWorker(int k, float agora);
    void DesenharSupervisao();

    struct Worker {
        BossProjectile* p = nullptr;
        float nasceu = 0.0f;
        int caixa = 0;
        float reinicio = -1.0f;   ///< quando volta; negativo = nao esta caido
    };
    struct Cast {
        BossProjectile* p = nullptr;
        float nasceu = 0.0f;
        int para = 0;
    };

    bool mNoElixir = false;
    std::string mAnuncioDoElixir;
    Variacao::Sorteio mSorteioDoElixir{0u};
    std::array<Elixir::Estrategia, 3> mEstrategias{};
    int mRodadaDoElixir = -1;
    std::array<Worker, Elixir::kWorkers> mWorkers{};
    std::vector<Cast> mCasts;
    float mProximoCast = 0.0f;
    int mUltimoCast = -1;
    float mTempoVarrendoNoElixir = 0.0f;
    int mParadaDoSupervisor = -1;          ///< a parada em curso, a cada 2 s
    int mEtapasNoPipe = 0;

    // ---- O event loop da final ----

    void AtualizarEventLoop(float deltaTime);
    void NovoScript(float agora);
    void DesenharEventLoop(float t);

    struct Ficha {
        BossProjectile* p = nullptr;
        float nasceu = 0.0f;
    };

    bool mNoLoop = false;
    std::string mAnuncioDoLoop;
    Variacao::Sorteio mSorteioDoLoop{0u};
    EventLoop::Degrau mDegrauDoLoop;
    std::vector<EventLoop::Linha> mScript;
    EventLoop::Agenda mAgenda;
    float mInicioDoScript = 0.0f;
    size_t mProximaExecucao = 0;
    std::vector<Ficha> mFichas;
};
