//
// A chefe da INF 221 - Engenharia de Software.
//

#pragma once

#include <array>
#include <string>
#include <vector>

#include "../Boss.h"
#include "../../../Attacks/MariaLucia/BalaDePrata.h"
#include "../../../Attacks/MariaLucia/Cascata.h"
#include "../../../Attacks/MariaLucia/Observer.h"
#include "../../../Attacks/Shared/Sprint.h"

class DrawLinhasComponent;

/**
 * @class MariaLucia
 * @brief A professora de Engenharia de Software (INF 221), que tambem da IHC.
 *
 * O BALANCEAMENTO DELA VIVE EM DADOS: as fases em
 * Assets/Attacks/marialucia/fases.json e as regras de projetil no regras.json
 * ao lado.
 *
 * A IDEIA DA CHEFE: as outras disciplinas do jogo sao OBJETOS - uma estrutura
 * de dados, uma linguagem, uma primitiva de concorrencia. Engenharia de
 * Software e um PROCESSO, e cada fase dela e um processo ou um padrao de
 * verdade:
 *
 *   fase 1  o MODELO CASCATA: cinco etapas em ordem, ela descendo a escada
 *           do diagrama (EscadaStrategy), e o que cada etapa entrega passando
 *           para a seguinte (Cascata.h): os pedidos viram classes, as classes
 *           viram codigo e bugs, os bugs que sobram viram divida;
 *   fase 2  os METODOS AGEIS: cinco sprints, e o cursor que arrasta as
 *           historias (SprintAttack);
 *   fase 3  o padrao OBSERVER: o aluno e o sujeito, e os olhos dela o assinam;
 *   final   NAO HA BALA DE PRATA (BalaDePrata.h): as promessas da industria
 *           que se desfazem em bugs, e o tiro do aluno como teste com usuario.

 *
 * O que mora aqui e a cascata da fase 1 em campo, e o SUJEITO da fase 3
 * (Observer.h): a cada quadro ela le para onde cada aluno esta indo e,
 * quando o estado muda - ou quando ninguem avisa por um tempo, o polling -,
 * puxa o gatilho dos avisos (Boss::Disparar). Desenha tambem as assinaturas,
 * uma linha de cada olho ate o aluno, e faz cada olho olhar para ele.
 */
class MariaLucia : public Boss {
public:
    explicit MariaLucia(Scene* scene);
    ~MariaLucia() override = default;

    void OnUpdate(float deltaTime) override;

    /// A fase do padrao Observer, a que decide a batalha.
    static constexpr const char* kFaseDoObserver = "StateThree";

    /// A repescagem: nao ha bala de prata.
    static constexpr const char* kFaseDaBala = "StateFinal";

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;
    void OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                              const std::string& stateName) override;

private:
    // ---- A LUTA SORTEADA ----------------------------------------------------
    //
    // Antes do primeiro quadro (Variacao.h): a ordem em que as classes da
    // cascata falam, o planning poker das sprints - escrito no placar de cada
    // uma -, e o relogio do sujeito do Observer, com o degrau do modo.

    void SortearALuta();

    /// A sprint em que a fase 2 esta agora (0 a 4), pelo relogio da fase e
    /// pelos fins registrados das etapas.
    [[nodiscard]] int SprintDaVez() const;

    bool mLutaSorteada = false;
    Cascata::Plano mPlanoDaCascata;
    Sprint::Poker mPoker = Sprint::kBaralho;
    Observer::Sujeito mSujeitoDaLuta;

    static constexpr const char* kFaseDosMetodos = "StateTwo";

    // ---- NAO HA BALA DE PRATA (a repescagem, BalaDePrata.h) ----------------

    void AtualizarBalas(float deltaTime);

    struct Bala {
        BossProjectile* p = nullptr;
        float saiu = 0.0f;     ///< no relogio da batalha (mRelogio)

        int promessa = 0;
    };
    std::vector<Bala> mBalas;
    std::vector<int> mOrdemDasPromessas;
    int mProximaPromessa = 0;
    float mProximaBala = 0.0f;
    bool mNaBala = false;
    BalaDePrata::Degrau mDegrauDaBala;
    std::string mNomeDaBala;
    std::string mTextoDaBala;
    float mTextoDaBalaPor = 0.0f;


    // ---- A CASCATA (fase 1) -------------------------------------------------


    void AtualizarCascata(float deltaTime);

    /// O que vale a fase toda e alem dela: a divida parada, que atravessa a
    /// troca de fase e so sai no tempo dela.
    void AtualizarDividas(float deltaTime);

    /// Dispara 'gatilho' em 'onde' e devolve os projeteis que nasceram.
    /// 'mirar': sai mirado no aluno.
    std::vector<BossProjectile*> DispararEm(const char* gatilho, Vector2 onde, bool mirar = false);

    /// Desfaz tudo o que a cascata pos em campo (o fim da fase).
    void DesmontarCascata();

    bool mDisparandoEm = false;
    bool mMirandoEm = false;
    Vector2 mOndeDisparar;
    std::vector<BossProjectile*> mCriados;

    bool mNaCascata = false;
    int mEtapaDaCascata = -1;
    std::array<BossProjectile*, Cascata::kEtapas.size()> mPlacas{};

    struct Pedido { BossProjectile* p = nullptr; int lugar = 0; };
    std::vector<Pedido> mPedidos;                 ///< no ar, a caminho do backlog
    std::vector<BossProjectile*> mBacklog;        ///< colados no quadro
    float mProximoPedido = 0.0f;

    struct Classe { BossProjectile* p = nullptr; Vector2 lugar; bool pronta = false; };
    std::vector<Classe> mClasses;
    DrawLinhasComponent* mDiagrama = nullptr;
    float mProximaMensagem = 0.0f;
    float mProximaCompilacao = 0.0f;
    size_t mClasseDaVez = 0;

    std::vector<BossProjectile*> mBugs;

    struct Divida { BossProjectile* p = nullptr; float ate = 0.0f; };
    std::vector<Divida> mDividas;
    float mRelogio = 0.0f;

    static constexpr float kRapidezDoPostIt = 7.0f;

    /// Tudo o que o sujeito faz num quadro: le os alunos, avisa, desenha.
    void AtualizarObserver(float deltaTime);

    /// Os olhos em campo: os projeteis no posto dos observadores.
    [[nodiscard]] std::vector<BossProjectile*> Observadores() const;

    /// O placar da fase 3: o nome do padrao, e o que se diz num aviso.
    void AnunciarOObserver();

    /// Um sujeito por aluno: em dupla, cada um avisa por si, e os avisos de um
    /// vao para ele (Boss::Disparar com alvo).
    std::vector<Observer::Sujeito> mSujeitos;
    bool mNoObserver = false;

    /// Quanto falta do piscar dos olhos (e do brilho das assinaturas) e do
    /// aviso no placar, depois do ultimo aviso.
    float mPiscando = 0.0f;
    float mNoPlacar = 0.0f;
    bool mUltimoFoiConsulta = false;
    std::string mNomeDoPadrao;

    /// As assinaturas. O ator que as guarda e da cena, criado na primeira vez
    /// que a fase 3 roda; a professora so as escreve.
    DrawLinhasComponent* mAssinaturas = nullptr;

    static constexpr float kPiscar = 0.2f;
    static constexpr float kAvisoNoPlacar = 0.9f;

    /// Atras dos projeteis (88 a 91) e na frente do placar (60).
    static constexpr int kOrdemDasAssinaturas = 85;

    static constexpr const char* kNoPlacarNotifica = "aluno.notify()";
    static constexpr const char* kNoPlacarConsulta = "polling: aluno.getEstado()";
};
