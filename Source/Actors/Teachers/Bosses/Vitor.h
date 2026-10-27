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

/**
 * @class Vitor
 * @brief O professor de Programacao Concorrente e Distribuida (INF 310).
 *
 * O BALANCEAMENTO DELE VIVE EM DADOS, como o dos outros: as fases em
 * Assets/Attacks/vitor/fases.json e as regras de projetil no regras.json ao
 * lado. O que mora aqui e o que dados nao sabem fazer: a fila do semaforo, a
 * janela do TCP, o fork pela nuvem.
 *
 * A IDEIA DO CHEFE: concorrencia e "varias coisas ao mesmo tempo", e e por isso
 * que a fase 3 dele se divide em dois corpos.
 *
 *   fase 1  SEMAFOROS           - as threads, a secao critica e a fila do P(s)
 *   fase 2  TCP                 - as nuvens, o roteador e o dente de serra
 *   fase 3  FORK                - dois processos trocando mensagens
 *   final   DEADLOCK            - o jantar dos filosofos
 *
 * A FASE 1 E A SINCRONIZACAO (Semaforo.h): cada thread e uma coluna de
 * instrucoes descendo ate a secao critica, no meio do campo. Quem nao tem o
 * semaforo para na borda e se empilha - a fila do P(s) -, e o V(s) de quem sai
 * acorda a primeira da fila: a pilha desce de uma vez, em leque. O professor
 * fica ao lado da coluna que ele vai acordar, e as correntes continuam: o
 * aluno acorrentado fica lento, e a coluna em cima dele fura a fila.
 *
 * E ELE TAMBEM DA REDES: na fase 2 as duas NUVENS em cima dele sao o
 * servidor, e cada chuva e uma janela TCP (Tcp.h). Os segmentos voam ate o
 * ROTEADOR no meio do campo, que encaminha ao aluno o que cabe e descarta o
 * resto num anel de cacos; a janela dobra no slow start, cresce de um na
 * evitacao, e cai a metade na perda - o dente de serra, desenhado num grafico
 * no canto. O Ponto Extra derruba as nuvens, e elas voltam num timeout, com
 * cwnd = 1. E e pela nuvem que ele se divide na fase 3: as duas se juntam em
 * cima dele, ele entra, e saem dois (ComecarOFork).
 *
 * A FASE 3 E A TROCA DE MENSAGENS (Mensagens.h): saem dois da nuvem, o pai e
 * o filho, e entre eles corre um canal em arco pelo campo. O original conduz
 * os dois corpos - o clone empresta uma fase so de gatilho, e nao anda nem
 * atira sozinho - e cada envelope corre pelo arco: a caixa postal do pai que
 * enche, o rendezvous do envio sincrono, a rpc que devolve um leque, e o
 * impasse dos dois enviando ao mesmo tempo.
 *
 * A FINAL E O DEADLOCK, no jantar dos filosofos (Filosofos.h): cinco em volta
 * da mesa, um garfo entre cada dois, e o professor no meio, de monitor. Os
 * garfos andam ate a mao de quem os pega, quem come ataca em leque, e quem
 * espera tem uma linha ate quem tem o garfo dele. Na rodada do deadlock as
 * esperas fecham um pentagono, que vira um cerco de projeteis ate o professor
 * matar um filosofo e o cerco estourar para fora.
 */
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
    // ---- O QUE AS FASES DIVIDEM ---------------------------------------------

    [[nodiscard]] SDL_FRect Campo() const;

    /// Dispara 'gatilho' em 'onde' e devolve os projeteis que nasceram (o
    /// mesmo do Luizinho). 'quantos' > 0 troca o numero de projeteis, 'centro'
    /// o meio do leque e 'abertura' a abertura dele.
    std::vector<BossProjectile*> DispararEm(const char* gatilho, Vector2 onde, int quantos = 0,
                                            std::optional<float> centro = std::nullopt,
                                            std::optional<float> abertura = std::nullopt);

    bool mDisparandoEm = false;
    Vector2 mOndeDisparar;
    int mQuantos = 0;
    std::optional<float> mCentroEm;
    std::optional<float> mAberturaEm;
    std::vector<BossProjectile*> mCriados;

    [[nodiscard]] Player* Aluno() const;
    [[nodiscard]] Vector2 PosicaoDoAluno() const;

    /**
     * @brief COMO O ALUNO ANDA: a velocidade media dos ultimos instantes, e
     * nao a do quadro. Quem segue alguem anda aos trancos - corre um quadro,
     * para dois -, e a velocidade do quadro diria 300 ou 0, nunca os 90 em
     * que ele anda de fato. Quem mira adiante (o roteador da fase 2) le esta.
     */
    void AcompanharOAluno(float deltaTime);
    Vector2 mVelocidadeDoAluno = Vector2::Zero;
    Vector2 mUltimaDoAluno = Vector2::Zero;
    bool mViuOAluno = false;
    static constexpr float kMemoriaDoAndar = 0.25f;

    /// O placar: o que a fase diz no arquivo e guardado na primeira vez, e
    /// volta quando a fase acaba.
    void Anunciar(const std::string& fase, const std::string& texto);
    void Restaurar(const std::string& fase);
    std::map<std::string, std::string> mAnunciosDoArquivo;

    /// Para onde o movimento "Conduzido" leva o professor.
    std::optional<Vector2> mAlvo;

    /// As linhas do campo, limpas a cada quadro e redesenhadas por quem
    /// estiver rodando.
    DrawLinhasComponent* mLinhas = nullptr;
    static constexpr int kOrdemDasLinhas = 80;

    // ---- A FASE 1: OS SEMAFOROS ---------------------------------------------

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

    /// Uma instrucao em campo: se ela ja saiu torta da faixa (na corrida).
    struct Instrucao {
        BossProjectile* p = nullptr;
        bool desviou = false;
    };

    /// Uma thread: quantas instrucoes ja mandou, quando manda a proxima, as
    /// que descem (ainda livres) e as paradas na pilha - ou girando, na espera
    /// ocupada. As que ja passaram pela porta nao sao mais dela.
    struct Thread {
        int mandadas = 0;
        float proximaEm = 0.0f;
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

    /// Onde o professor fica: ao lado da coluna que ele vai acordar, entre ela
    /// e a vizinha - fora das duas colunas, e fora da pilha.
    static constexpr float kAoLadoDaColuna = 110.0f;
    static constexpr float kAlturaDoProfessor = 200.0f;

    /// A perseguicao da rajada de quem acorda com o aluno acorrentado: preso,
    /// ele deixa de estar seguro ao lado da coluna (a regra "paralelo" das
    /// tarefas de antes, que fazia o mesmo).
    static constexpr float kAtrasoDaPerseguicao = 0.2f;
    static constexpr float kForcaDaPerseguicao = 2.4f;
    static constexpr float kDuracaoDaPerseguicao = 2.2f;

    static constexpr int kOrdemDaSecao = 52;
    static constexpr int kOrdemDosNomes = 57;
    // ---- A FASE 3: A TROCA DE MENSAGENS ---------------------------------------

    void AtualizarMensagens();
    void MontarMensagens();
    void DesmontarMensagens();
    void ComecarRodadaDasMensagens(int k);
    void ExecutarMensagem(const Mensagens::Evento& e);
    void MoverEnvelopes(float u);
    void DesenharCanal();

    /// O outro corpo do fork, o filho - o clone em campo, ou nulo.
    [[nodiscard]] Vitor* Filho() const;

    /// Onde esta o processo agora: o pai e este corpo; o filho, o clone.
    [[nodiscard]] Vector2 PosicaoDe(int processo) const;

    bool mNasMensagens = false;
    int mRodadaDasMensagens = -1;
    float mInicioDasMensagens = 0.0f;
    std::vector<Mensagens::Evento> mRoteiroDasMensagens;
    size_t mProximaMensagem = 0;
    float mRelogioDasMensagens = 0.0f;
    int mChamadaDaRpc = 0;
    int mRecebidas = 0;
    bool mEsperandoRpc = false;
    bool mEstourou = false;

    /// Um envelope no canal: de quem saiu, quando, quanto leva, e se para na
    /// porta (o impasse).
    struct Envelope {
        BossProjectile* p = nullptr;
        int de = Mensagens::kPai;
        float saiu = 0.0f;
        float voo = 1.0f;
        bool paraNaPorta = false;
    };
    std::vector<Envelope> mEnvelopes;

    /// "pai" e "filho", embaixo de cada corpo.
    std::array<EtiquetaDeCodigo*, 2> mNomesDosProcessos{};
    static constexpr float kAbaixoDoProcesso = 78.0f;
    static constexpr float kAberturaPorDado = 9.0f;

    // ---- A FINAL: O JANTAR DOS FILOSOFOS --------------------------------------

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

    /// OS NOMES DOS FILOSOFOS FICAM POR CIMA DO PROFESSOR (100), e so eles:
    /// ele vai ate quem vigia, e por baixo dele o nome de quem come sumia -
    /// justo o que o placar manda ler.
    static constexpr int kOrdemDosFilosofos = 102;

    /// Os garfos andam a kVelocidadeDoGarfo ate onde devem estar.
    static constexpr float kVelocidadeDoGarfo = 520.0f;

    /// O professor vai ate perto de quem ele vigia (kPertoDoEscolhido do
    /// caminho do meio da mesa ate o filosofo), e sem ninguem para vigiar
    /// balanca no meio.
    ///
    /// ELE NAO PODE MORAR NO MEIO: o aluno parado embaixo do meio da mesa
    /// atirava nele a fase inteira - 153 acertos na final sozinha - e passava
    /// mesmo tomando dez garfadas.
    static constexpr float kBalancoNaMesa = 60.0f;
    static constexpr float kRitmoDoBalanco = 1.0f;
    static constexpr float kPertoDoEscolhido = 0.45f;
    static constexpr float kAbertoDaGarfada = 16.0f;

    // ---- A FASE 2: AS NUVENS E O TCP ----------------------------------------
    //
    // Pouco depois de a fase comecar o Vitor forma as duas nuvens (o gatilho
    // "nuvens"), e dali elas trabalham num RTT (Tcp.h): processam, e no fim do
    // processamento mandam a janela - os segmentos voam ate o roteador, que
    // encaminha ao aluno o que cabe e descarta o resto. SO O CORPO ORIGINAL:
    // a nuvem e do Vitor, e nao de cada copia dele.

    void AtualizarNuvens(float deltaTime);
    void MontarRede();
    void DesmontarRede();
    void MandarSegmentos(float agora);
    void ChegarNoRoteador(float agora);
    void Encaminhar(float agora);
    void DesenharGrafico();

    /// As nuvens em campo: os projeteis no posto das nuvens.
    [[nodiscard]] std::vector<BossProjectile*> Nuvens() const;

    /// Puxa um gatilho das nuvens.
    void DispararNuvem(const char* gatilho);

    /// O angulo, em graus, de 'de' ate 'para'.
    static float Angulo(Vector2 de, Vector2 para);

    /// @brief O angulo de 'de' ate onde o aluno estara quando um projetil a
    /// 'rapidez' px/s chegar la, antecipando 'quanto' do caminho que ele anda
    /// (Tcp::Mira, com mVelocidadeDoAluno). 0 e onde ele esta.
    [[nodiscard]] float AnguloAdiante(Vector2 de, float rapidez, float quanto) const;

    [[nodiscard]] Vector2 PosicaoDoRoteador() const;

    /// O fork pela nuvem, no corpo original: junta as nuvens em cima dele,
    /// manda os dois corpos para dentro dela, e a desfaz quando eles saem.
    void ComecarOFork();
    void AtualizarANuvemDoFork();
    void SairDaNuvem();

    /**
     * @brief Um corpo entra no fork pela nuvem (Nuvem::Fork): fica em 'onde',
     * some ou nasce dentro dela, e quando ela abre anda ate 'destino'. Ate la
     * nao anda sozinho nem ataca - mas o colisor fica: o tiro que entra na
     * nuvem acerta quem esta dentro dela. O original chama isto nos dois.
     */
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
    float mSemNuvensHa = 0.0f;     ///< ha quanto tempo as nuvens formadas sumiram
    std::string mNomeDaFaseDasNuvens;

    /// A CONEXAO e a janela em voo. A janela sai segmento a segmento
    /// (Tcp::kEntreSegmentos), alternando as nuvens; quando o ultimo chegou ao
    /// roteador - ou foi descartado -, os ACKs voltam e a conexao reage.
    Tcp::Conexao mConexao;
    Tcp::Janela mJanela;
    bool mMandando = false;
    int mMandados = 0;
    float mInicioDaJanela = 0.0f;
    bool mJanelaReconhecida = true;

    struct Segmento {
        BossProjectile* p = nullptr;
        bool perdido = false;
        bool retransmitido = false;
    };
    std::vector<Segmento> mEmVoo;

    /// A FILA DO ROTEADOR: os que ele ainda vai encaminhar (se cada um e
    /// retransmitido), e quando sai o proximo.
    std::vector<bool> mFila;
    int mEncaminhados = 0;
    int mRecebidosNaJanela = 0;
    float mProximoEncaminhado = 0.0f;
    int mDescartes = 0;

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

    /// O FORK: os tempos dele, e o tamanho da nuvem juntada - as duas viram
    /// uma maior que o professor: o quadro da nuvem tem 32 px e o dele 128, e
    /// a 5x ela fica mais larga que ele.
    static constexpr Nuvem::Fork kFork{};
    static constexpr float kEscalaDaNuvemJunta = 5.0f;
    static constexpr const char* kNoFork = "nuvem: fork()";
    std::string mNomeDaFaseDoFork;
};
