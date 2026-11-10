//
// O chefe da INF 310 - Programacao Concorrente e Distribuida.
//

#pragma once

#include <array>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <SDL_rect.h>

#include "../Boss.h"
#include "../../../Attacks/Nuvem.h"
#include "../../../Attacks/Semaforo.h"
#include "../../../Attacks/Tcp.h"
#include "../../../Attacks/Filosofos.h"
#include "../../../Attacks/Mensagens.h"

class BossProjectile;
class DrawLinhasComponent;
class EtiquetaDeCodigo;
class PainelDeCodigo;
class Player;

/// O professor de Programacao Concorrente e Distribuida (INF 310).
class Vitor : public Boss {
public:
    explicit Vitor(Scene* scene);
    ~Vitor() override = default;

    void OnUpdate(float deltaTime) override;
    [[nodiscard]] std::optional<Vector2> AlvoDoMovimento() const override;
    void AoTerminarABatalha() override;

    /// A fase das nuvens, e a do fork, que a nuvem abre (Nuvem.h).
    static constexpr const char* kFaseDasNuvens = Nuvem::kFaseDasNuvens;
    static constexpr const char* kFaseDoFork = Nuvem::kFaseDoFork;

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;
    void OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                              const std::string& stateName) override;

private:
    // ---- O que as fases dividem ----


    /// Dispara 'gatilho' em 'onde' e devolve os projeteis que nasceram (o mesmo do Luizinho).
    std::vector<BossProjectile*> DispararEm(const char* gatilho, Vector2 onde, int quantos = 0,
                                            std::optional<float> centro = std::nullopt,
                                            std::optional<float> abertura = std::nullopt,
                                            float rapidez = 1.0f);

    bool mDisparandoEm = false;
    Vector2 mOndeDisparar;
    int mQuantos = 0;
    std::optional<float> mCentroEm;
    std::optional<float> mAberturaEm;
    std::vector<BossProjectile*> mCriados;


    /// Como o aluno anda: a velocidade media dos ultimos instantes, e nao a do quadro.
    void AcompanharOAluno(float deltaTime);
    Vector2 mVelocidadeDoAluno = Vector2::Zero;
    Vector2 mUltimaDoAluno = Vector2::Zero;
    bool mViuOAluno = false;
    static constexpr float kMemoriaDoAndar = 0.25f;


    /// Para onde o movimento "Conduzido" leva o professor.
    std::optional<Vector2> mAlvo;

    /// O acaso de cada fase nesta luta - o escalonador, a rede, a latencia, o jantar -, da semente
    /// da batalha. Criado na primeira vez e nunca recomecado: a volta seguinte pela fase continua a
    /// sequencia, e nao a repete.
    Variacao::Sorteio& Acaso(const std::string& fase);
    std::map<std::string, Variacao::Sorteio> mAcasos;

    /// As linhas do campo, limpas a cada quadro e redesenhadas por quem
    /// estiver rodando.
    DrawLinhasComponent* mLinhas = nullptr;
    static constexpr int kOrdemDasLinhas = 80;

    // ---- A fase 1: os semaforos ----

    void AtualizarSemaforos();
    void MontarSemaforos();
    void DesmontarSemaforos();
    void ComecarRodadaDoSemaforo(int k);
    void SoltarAsPilhas(bool matando);
    void Mandar(int thread);
    void Acordar(int thread);
    void MoverInstrucoes(float t);
    void PintarThreads();
    void DesenharSemaforos();

    bool mNosSemaforos = false;
    int mRodadaDoSemaforo = -1;
    float mInicioDaRodada = 0.0f;
    Semaforo::Escalonador mEscalonador;
    std::vector<float> mColunas;

    /// O que esta batalha sorteou para a fase (a ordem das rodadas, o s do
    /// contador, o passo das colunas) e o degrau do modo. Sorteados ao montar.
    Semaforo::Plano mPlanoDosSemaforos = Semaforo::PlanoDeSempre();
    Semaforo::Degrau mDegrauDosSemaforos;

    /// Uma instrucao em campo: se ela ja saiu torta da faixa (na corrida).
    struct Instrucao {
        BossProjectile* p = nullptr;
        bool desviou = false;
    };

    /// Uma thread: quantas instrucoes ja mandou, quando manda a proxima (no ritmo dela), as que
    /// descem (ainda livres) e as paradas na pilha - ou girando, na espera ocupada.
    struct Thread {
        int mandadas = 0;
        float proximaEm = 0.0f;
        Semaforo::Ritmo ritmo;
        std::vector<Instrucao> descendo;
        std::vector<BossProjectile*> paradas;
        EtiquetaDeCodigo* nome = nullptr;
    };
    std::vector<Thread> mThreads;

    /// As que passaram pela porta e ainda estao descendo: so a corrida as
    /// entorta, uma vez, ao sair da faixa.
    std::vector<Instrucao> mPassando;

    PainelDeCodigo* mSecao = nullptr;
    EtiquetaDeCodigo* mCodigoDaSecao = nullptr;

    /// O aluno estava acorrentado no quadro anterior? A coluna dele fura a
    /// fila na hora em que ele e pego, uma vez por corrente.
    bool mAlunoPreso = false;

    /// Onde o professor fica: ao lado da coluna que ele vai acordar, no meio entre ela e a vizinha
    /// - fora das duas colunas, e fora da pilha.
    static constexpr float kAoLadoDaColuna = Semaforo::kEntreThreads / 2.0f;
    static constexpr float kAlturaDoProfessor = 200.0f;

    static constexpr float kAtrasoDaPerseguicao = 0.2f;
    static constexpr float kForcaDaPerseguicao = 2.4f;
    static constexpr float kDuracaoDaPerseguicao = 2.2f;

    static constexpr int kOrdemDaSecao = 52;
    static constexpr int kOrdemDosNomes = 57;
    // ---- A fase 3: a troca de mensagens ----

    void AtualizarMensagens();
    void MontarMensagens();
    void DesmontarMensagens();
    void ComecarRodadaDasMensagens(int k);
    void ExecutarMensagem(const Mensagens::Evento& e);
    void MoverEnvelopes(float u);
    void DesenharCanal();

    /// O outro corpo do fork, o servidor - o clone em campo, ou nulo.
    [[nodiscard]] Vitor* Servidor() const;

    [[nodiscard]] Vector2 PosicaoDe(int processo) const;

    bool mNasMensagens = false;
    int mRodadaDasMensagens = -1;
    float mInicioDasMensagens = 0.0f;
    std::vector<Mensagens::Evento> mRoteiroDasMensagens;

    /// A fase desta batalha (a ordem, as contas da rpc, os lugares) e o degrau
    /// do modo. Sorteados quando os dois saem da nuvem.
    Mensagens::Plano mPlanoDasMensagens = Mensagens::PlanoDeSempre();
    Mensagens::Degrau mDegrauDasMensagens;
    [[nodiscard]] Mensagens::Rodada RodadaDasMensagens(int k) const;
    size_t mProximaMensagem = 0;
    float mRelogioDasMensagens = 0.0f;
    int mChamadaDaRpc = 0;
    bool mEsperandoRpc = false;
    bool mEstourou = false;

    /// Um envelope no canal: de quem saiu, quando, quanto leva, e se para na
    /// porta (o impasse).
    struct Envelope {
        BossProjectile* p = nullptr;
        int de = Mensagens::kCliente;
        float saiu = 0.0f;
        float voo = 1.0f;
        bool paraNaPorta = false;
    };
    std::vector<Envelope> mEnvelopes;

    /// "cliente" e "servidor", embaixo de cada corpo.
    std::array<EtiquetaDeCodigo*, 2> mNomesDosProcessos{};
    static constexpr float kAbaixoDoProcesso = 78.0f;
    static constexpr float kAberturaPorDado = 9.0f;

    // ---- A final: o jantar dos filosofos ----

    void AtualizarJantar(float deltaTime);
    void MontarJantar();
    void DesmontarJantar();
    void ComecarRodadaDoJantar(int k);
    void ExecutarNoJantar(const Filosofos::Passo& p);
    void MoverGarfos(float deltaTime);
    void PintarFilosofos();
    void DesenharJantar();
    void FecharCerco(float u);
    void EstourarCerco();

    [[nodiscard]] Vector2 NoCampo(Filosofos::Ponto p) const;

    /// O filosofo mais perto do aluno: e quem o professor escolhe - a vitima
    /// do kill -9, e quem o garcom tira da mesa.
    [[nodiscard]] int FilosofoMaisPerto() const;

    bool mNoJantar = false;
    int mRodadaDoJantar = -1;
    float mInicioDoJantar = 0.0f;
    int mEscolhido = 0;
    Filosofos::Mesa mMesa;

    /// O jantar desta batalha (a ordem das solucoes, quem se senta, de onde o
    /// jantar comeca) e o degrau do modo. Sorteados ao por a mesa.
    Filosofos::Plano mPlanoDoJantar = Filosofos::PlanoDeSempre();
    Filosofos::Degrau mDegrauDoJantar;
    std::vector<Filosofos::Passo> mPassos;
    size_t mProximoPasso = 0;
    float mRelogioDoJantar = 0.0f;

    std::array<EtiquetaDeCodigo*, Filosofos::kFilosofos> mNomesDosFilosofos{};
    std::array<BossProjectile*, Filosofos::kFilosofos> mGarfos{};
    std::array<float, Filosofos::kFilosofos> mProximaGarfada{};

    /// O cerco do deadlock: as esperas ja postas, e quantas ja sairam.
    std::vector<BossProjectile*> mCerco;
    int mPostasNoCerco = 0;
    float mInicioDoCerco = 0.0f;
    bool mCercoAberto = false;

    /// Os nomes dos filosofos ficam por cima do professor (100), e so eles: ele vai ate quem vigia,
    /// e por baixo dele o nome de quem come sumia.
    static constexpr int kOrdemDosFilosofos = 102;

    /// Os garfos andam a kVelocidadeDoGarfo ate onde devem estar.
    static constexpr float kVelocidadeDoGarfo = 520.0f;

    /// O professor vai ate perto de quem ele vigia (kPertoDoEscolhido do caminho do meio da mesa
    /// ate o filosofo), e sem ninguem para vigiar balanca no meio.
    static constexpr float kBalancoNaMesa = 60.0f;
    static constexpr float kRitmoDoBalanco = 1.0f;
    static constexpr float kPertoDoEscolhido = 0.45f;
    static constexpr float kAbertoDaGarfada = 16.0f;

    // ---- A fase 2: as nuvens e o TCP ----

    void AtualizarNuvens(float deltaTime);
    void MontarRede();
    void DesmontarRede();
    void MandarSegmentos(float agora);
    void ChegarNoRoteador(float agora);
    void Encaminhar(float agora);
    /// O anel de cacos de um segmento perdido em 'onde': no roteador cheio, ou no caminho (o ruido).
    void Estilhacar(Vector2 onde);
    void DesenharGrafico();

    /// As nuvens em campo: os projeteis no posto das nuvens.
    [[nodiscard]] std::vector<BossProjectile*> Nuvens() const;

    /// Puxa um gatilho das nuvens.
    void DispararNuvem(const char* gatilho);

    /// O angulo, em graus, de 'de' ate 'para'.
    static float Angulo(Vector2 de, Vector2 para);

    [[nodiscard]] float AnguloAdiante(Vector2 de, float rapidez, float quanto) const;

    [[nodiscard]] Vector2 PosicaoDoRoteador() const;

    /// O fork pela nuvem, no corpo original: junta as nuvens em cima dele,
    /// manda os dois corpos para dentro dela, e a desfaz quando eles saem.
    void ComecarOFork();
    void AtualizarANuvemDoFork();
    void SairDaNuvem();

    /// Um corpo entra no fork pela nuvem (Nuvem::Fork): fica em 'onde', some ou nasce dentro dela,
    /// e quando ela abre anda ate 'destino'.
    void EntrarNaNuvem(Vector2 onde, Vector2 destino, bool nasceuDentro);

    /// Ha quanto tempo o corpo esta no fork (negativo: nao esta), de onde ele
    /// sai e para onde vai.
    float mTempoNoFork = -1.0f;
    bool mNasceuDentro = false;
    Vector2 mSaiDe;
    Vector2 mVaiPara;
    float mEscalaDaNuvem = 1.0f;

    /// O RTT e o ciclo da nuvem, com os tempos do TCP.
    Nuvem::Ciclo mChuva{Tcp::kProcessa, Tcp::kDescansa, 0.0f};
    bool mNasNuvens = false;
    bool mNuvensFormadas = false;
    float mTempoNasNuvens = 0.0f;
    float mSemNuvensHa = 0.0f;
    std::string mNomeDaFaseDasNuvens;

    /// A conexao e a janela em voo.
    Tcp::Conexao mConexao;

    /// A rede desta batalha (a capacidade, o ssthresh, onde fica o roteador) e
    /// o degrau do modo. Sorteados quando a fase abre.
    Tcp::Plano mPlanoDaRede = Tcp::PlanoDeSempre();
    Tcp::Degrau mDegrauDaRede;
    Tcp::Janela mJanela;
    bool mMandando = false;
    int mMandados = 0;
    float mInicioDaJanela = 0.0f;
    bool mJanelaReconhecida = true;

    /// Um segmento em voo: o descartado no roteador cheio, o retransmitido, e o que o ruido leva -
    /// esse estoura a 'quebraEm' px de onde saiu.
    struct Segmento {
        BossProjectile* p = nullptr;
        bool perdido = false;
        bool retransmitido = false;
        bool ruido = false;
        Vector2 de;
        float quebraEm = 0.0f;
    };
    std::vector<Segmento> mEmVoo;

    /// A fila do roteador: os que ele ainda vai encaminhar (se cada um e retransmitido), e quando
    /// sai o proximo.
    std::vector<bool> mFila;
    int mRecebidosNaJanela = 0;
    float mProximoEncaminhado = 0.0f;

    /// O que o placar diz ate a janela seguinte sair: o estado da conexao, ou
    /// a perda, ou o timeout.
    std::string mTextoDaConexao;
    bool mAvisoPendente = false;

    /// O que o grafico ja desenhou: o cwnd de cada janela mandada, e onde a
    /// conexao recomecou (o timeout).
    std::vector<int> mJanelasDoGrafico;

    PainelDeCodigo* mRoteador = nullptr;
    EtiquetaDeCodigo* mFilaDoRoteador = nullptr;
    PainelDeCodigo* mQuadroDoGrafico = nullptr;
    EtiquetaDeCodigo* mNomeDoGrafico = nullptr;

    static constexpr const char* kForaDoAr = "nuvem: fora do ar";

    /// O fork: os tempos dele, e o tamanho da nuvem juntada - as duas viram uma maior que o
    /// professor.
    static constexpr Nuvem::Fork kFork{};
    static constexpr float kEscalaDaNuvemJunta = 5.0f;
    static constexpr const char* kNoFork = "nuvem: fork()";
    std::string mNomeDaFaseDoFork;
};
