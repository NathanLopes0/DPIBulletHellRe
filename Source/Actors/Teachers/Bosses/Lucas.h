//
// O chefe da INF 321 - Programacao Web.
//

#pragma once

#include <array>
#include <optional>
#include <string>
#include <vector>

#include "../Boss.h"
#include "../../../Attacks/Lucas/Pagina.h"
#include "../../../Attacks/Lucas/Elixir.h"
#include "../../../Attacks/Lucas/Estilo.h"
#include "../../../Attacks/Lucas/EventLoop.h"
#include "../../../Attacks/Shared/Flex.h"

class BossProjectile;
class DrawLinhasComponent;

/**
 * @class Lucas
 * @brief O professor de Programacao Web (INF 321).
 *
 * A IDEIA DO CHEFE: ele da a materia na ordem classica - HTML, CSS, JavaScript
 * - e no fim ensina o que ele gosta de verdade, que e Elixir no back-end. A
 * batalha conta a mesma historia. As duas primeiras fases sao o front; a
 * terceira, a que decide, e o Elixir; a final volta ao front, agora assincrono.
 *
 *   fase 1  HTML        uma pagina renderizada no navegador: o layout fica
 *                       desenhado, e cada elemento - button, img, marquee,
 *                       table, a, input - ataca do seu jeito (Pagina.h)
 *   fase 2  CSS         o DevTools: uma grade de cartoes que cada regra CSS do
 *                       placar transforma inteira - gap, rotate, scale,
 *                       display: none, animation, position: fixed (Estilo.h)
 *   fase 3  Elixir      a arvore de supervisao: workers que recebem casts,
 *                       processam e atiram, e que o supervisor reinicia pela
 *                       estrategia da rodada (Elixir.h)
 *   final   JavaScript  o event loop: scripts que passam pela pilha, timers
 *                       nas Web APIs, as filas de microtarefas e de tarefas -
 *                       e cada callback que sai de uma fila ataca (EventLoop.h)
 *
 * O BALANCEAMENTO VIVE EM DADOS, como o dos outros: Assets/Attacks/lucas/. O
 * que mora aqui e o que dados nao sabem fazer: a pagina que renderiza, a
 * grade do DevTools, a arvore de supervisao e o event loop.
 */
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
    // ---- A LUTA SORTEADA ----------------------------------------------------
    //
    // Antes do primeiro quadro (Variacao.h): de onde saem as paginas da fase 1
    // e as regras CSS da fase 2 (Estilo::Proxima), e o degrau do modo.

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
    [[nodiscard]] Vector2 PosicaoDoAluno() const;

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

    // ---- A PAGINA DA FASE 1 -----------------------------------------------------
    //
    // Um elemento a cada Pagina::Degrau::porElemento, na ordem da pagina; cada
    // um vive o tempo do ataque dele (Renderizado) - o cursor que voa ate o
    // botao, o spinner que gira, as linhas da tabela, as letras do input. O
    // professor fica no header, em cima do elemento que renderiza. Acabada a
    // pagina, o F5 apaga os elementos e vem outra.

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

    // ---- O DEVTOOLS DA FASE 2 ----------------------------------------------------
    //
    // A grade de cartoes fica no posto "grade", e o Lucas os move: a cada
    // quadro cada cartao vai ate o lugar que o CSS de agora lhe da
    // (Estilo::Posicoes), centrado embaixo do professor, que varre o alto. A
    // cada Estilo::Degrau::porRegra, uma regra nova. O cartao que falta - o da
    // linha que voltou do display: none, ou a grade que caiu no position:
    // fixed - nasce do professor e voa ate o lugar.

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

    // ---- A ARVORE DE SUPERVISAO DA FASE 3 -----------------------------------------
    //
    // Os workers ficam no posto "supervisao", embaixo do professor, e o Lucas
    // os move. A cada Elixir::Degrau::entreMensagens um cast corre pela linha
    // ate um worker; a caixa cheia vira um handle_cast, o leque. O worker que
    // cai e um crash: termina quem a estrategia da rodada manda (Elixir::
    // Reiniciam), cada outro num anel, e todos voltam depois de kReinicio.

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

    // ---- O EVENT LOOP DA FINAL ---------------------------------------------------------
    //
    // Cada linha do script vira uma ficha no posto "eventloop", e o Lucas a
    // leva para onde o loop a poe naquele instante (EventLoop::Onde): a pilha,
    // as Web APIs, uma das filas, a pilha de novo. O que roda na pilha ataca
    // de la (EventLoop::Agenda); o professor e o loop, orbitando em volta.

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
