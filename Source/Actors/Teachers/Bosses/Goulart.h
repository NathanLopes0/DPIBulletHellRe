//
// O chefe da INF 451 - Sistemas Operacionais.
//

#pragma once

#include <array>
#include <deque>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <SDL_pixels.h>

#include "../Boss.h"
#include "../../../Attacks/Disco.h"
#include "../../../Attacks/Escalonador.h"
#include "../../../Attacks/Paginacao.h"
#include "../../../Attacks/Virtualizacao.h"

class BossProjectile;
class DrawLinhasComponent;

/// O professor de Sistemas Operacionais (INF 451). Cada prova e um pedaco do sistema:
///   1. os processos - o escalonador Round Robin: a fila de prontos, a CPU, a E/S, o fork e o exit;
///   2. a memoria - a paginacao: a MMU no placar, a tabela de paginas, o relogio de molduras e o
///      thrashing;
///   3. a entrada e saida - o braco do disco: FCFS, SSTF e o elevador;
///   e a repescagem, a virtualizacao: o hipervisor, as VMs, o trap, a migracao e a elasticidade.
///
/// Tudo anda pelo relogio do sistema, e nao pelo que esta em campo: o Ponto Extra limpa a tela, e
/// o escalonador, a memoria, o disco e a nuvem continuam - e o que eles mostram volta.
class Goulart : public Boss {
public:
    explicit Goulart(Scene* scene);
    ~Goulart() override = default;

    void OnUpdate(float deltaTime) override;
    [[nodiscard]] std::optional<Vector2> AlvoDoMovimento() const override;
    void OnDerrubou(BossProjectile* qual) override;
    void AoTerminarABatalha() override;

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;
    void OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                              const std::string& stateName) override;

private:
    // ---- A luta sorteada ----

    void SortearALuta();
    bool mLutaSorteada = false;
    Variacao::Sorteio mSorteio{0u};
    Escalonador::Degrau mDegrauDoEscalonador;
    Paginacao::Degrau mDegrauDaPaginacao;
    Disco::Degrau mDegrauDoDisco;
    Virtualizacao::Degrau mDegrauDaNuvem;

    // ---- O que as fases dividem ----

    /// Dispara 'gatilho' em 'onde' e devolve os projeteis que nasceram. 'quantos', 'centro'
    /// (graus), 'abertura' (graus) e 'velocidade' trocam o que fases.json diz, quando dados.
    std::vector<BossProjectile*> DispararEm(const char* gatilho, Vector2 onde, int quantos = 0,
                                            std::optional<float> centro = std::nullopt,
                                            std::optional<float> abertura = std::nullopt,
                                            std::optional<float> velocidade = std::nullopt);
    bool mDisparandoEm = false;
    Vector2 mOndeDisparar;
    int mQuantos = 0;
    std::optional<float> mCentroEm;
    std::optional<float> mAberturaEm;
    std::optional<float> mVelocidadeEm;
    std::vector<BossProjectile*> mCriados;

    /// O primeiro que nasceu, ja com a cara e a escala. Com 'velocidade', nasce andando (o
    /// WaveAttack poe a do disparo no primeiro quadro do projetil).
    BossProjectile* Um(const char* gatilho, Vector2 onde, const std::string& cara, float escala = 1.0f,
                       std::optional<Vector2> velocidade = std::nullopt);

    [[nodiscard]] float AnguloAteOAluno(Vector2 de) const;
    [[nodiscard]] static bool Vivo(const BossProjectile* p);
    static void Matar(BossProjectile*& p);
    static void Cara(BossProjectile* p, const std::string& cara);
    /// Leva o projetil 'p' na direcao de 'para', sem passar dele: o cartao que anda ate a vaga.
    static void Levar(BossProjectile* p, Vector2 para, float deltaTime, float rapidez = 9.0f);

    std::string mFase;
    void Desmontar();

    DrawLinhasComponent* mLinhas = nullptr;
    static constexpr int kOrdemDasLinhas = 82;
    void Linha(Vector2 de, Vector2 ate, SDL_Color cor);
    void Retangulo(Vector2 centro, float largura, float altura, SDL_Color cor);
    /// Uma faixa grossa: 'linhas' linhas lado a lado, de 'de' a 'ate' em x, na altura 'y'.
    void Faixa(float de, float ate, float y, int linhas, SDL_Color cor);

    void PlacarDaEtapa(const std::string& fase, size_t etapa, const std::string& texto);
    void PlacarDaFase(const std::string& fase, const std::string& texto);
    /// Um texto no placar por um instante: fork, kill -9, falta de pagina...
    void Avisar(const std::string& texto, float duracao = 1.1f);
    std::string mAviso;
    float mAvisoAte = -1.0f;
    float mRelogio = 0.0f;

    std::optional<Vector2> mAlvo;

    // ---- Fase 1: o escalonador ----

    void AtualizarEscalonador(float t, float deltaTime);
    void DesmontarEscalonador();
    void Forkar(float t);
    void FecharFatia(int pid, float t);
    [[nodiscard]] Vector2 LugarNaCpu() const;
    [[nodiscard]] Vector2 LugarNaFila(int i) const;
    [[nodiscard]] Vector2 LugarNaES(int i) const;
    [[nodiscard]] Vector2 LugarDoTitulo(int k) const;
    /// A faixa 'f' do campo: as duas bordas, de cima a baixo, na cor 'cor'; tracejada, a do proximo.
    void Faixa(int f, SDL_Color cor, bool tracejada);
    std::optional<Escalonador::RoundRobin> mRR;
    std::map<int, BossProjectile*> mCartoes;
    int mUltimoPid = 0;
    int mForks = 0;
    float mProximoFork = 0.0f;
    float mProximaGota = 0.0f;
    std::array<BossProjectile*, Escalonador::kTitulos.size()> mTitulos{};
    /// O grafico de Gantt da fase: cada fatia de CPU, de quem foi e de quando a quando.
    struct Fatia {
        int pid = 0;
        float de = 0.0f;
        float ate = -1.0f;   ///< -1: ainda roda
    };
    std::vector<Fatia> mGantt;

    // ---- Fase 2: a paginacao ----

    void AtualizarPaginacao(float t, float deltaTime);
    void DesmontarPaginacao();
    [[nodiscard]] Vector2 CentroDoRelogio() const;
    [[nodiscard]] Vector2 LugarDaMoldura(int m) const;
    [[nodiscard]] Vector2 LugarDaEntrada(int p) const;
    /// A coluna da pagina 'p' desaba: a cortina comeca em 'inicio' (antes, a coluna fica marcada).
    void Despejar(int p, float agora, float inicio);
    std::optional<Paginacao::Relogio> mMemoria;
    std::vector<BossProjectile*> mMolduras;
    std::array<BossProjectile*, Paginacao::kPaginas> mEntradas{};
    int mEtapaDaPaginacao = -1;
    int mPaginaDoAluno = -1;
    float mProximaLeitura = 0.0f;
    float mProximoAcesso = 0.0f;
    /// A leitura do professor: de quanto em quanto, qual pagina e quando - a linha ate a entrada dela.
    float mProximaDoProfessor = 0.0f;
    int mPaginaDoProfessor = -1;
    float mProfessorLeuEm = -1.0f;
    /// A pagina 'p' vem do disco para a moldura 'm': o cartao voa do professor ate ela.
    void TrazerPagina(int p, int m);
    std::deque<bool> mUltimasFaltas;   ///< a janela do thrashing: true e falta
    std::string mUltimaReferencia;
    float mFaltouEm = -1.0f;
    /// A varredura do ponteiro na falta de pagina, que a tela mostra passo a passo.
    struct Varredura {
        std::vector<int> passos;
        int moldura = -1;
        float desde = 0.0f;
    };
    std::optional<Varredura> mVarredura;
    /// A cortina de cada pagina despejada.
    struct Cortina {
        int pagina = 0;
        float marcada = 0.0f;   ///< desde quando a coluna esta marcada
        float inicio = 0.0f;    ///< quando comeca a cair
        float fim = 0.0f;
        float proximo = 0.0f;
    };
    std::vector<Cortina> mCortinas;

    // ---- Fase 3: o braco do disco ----

    void AtualizarDisco(float t, float deltaTime);
    void DesmontarDisco();
    [[nodiscard]] float XDaTrilha() const;
    [[nodiscard]] float LarguraDaTrilha() const;
    [[nodiscard]] float YDaTrilha() const;
    struct Pedido {
        BossProjectile* p = nullptr;
        int cilindro = 0;
        float chegou = 0.0f;
    };
    std::vector<Pedido> mPedidos;
    float mProximoPedido = 0.0f;
    /// O pedido (ou a ponta, no elevador) com que o braco se comprometeu.
    BossProjectile* mPedidoDoBraco = nullptr;
    int mAlvoDoBraco = -1;
    int mEtapaDoBraco = -1;
    int mSentido = -1;
    int mEtapaDoDisco = -1;
    int mAndou = 0;
    int mUltimoCilindro = -1;
    int mCabecaNoInicio = 0;
    /// Os atendidos da etapa, com a hora de chegada: o placar compara com o FCFS deles.
    std::vector<std::pair<float, int>> mAtendidos;
    /// O bloco lido cai na coluna do pedido: uma cortina estreita, mais larga e longa no faminto.
    struct Leitura {
        float x = 0.0f;
        float meia = 0.0f;      ///< meia largura da coluna
        float fim = 0.0f;
        float proximo = 0.0f;
    };
    std::vector<Leitura> mLeituras;

    // ---- A repescagem: a nuvem ----

    void AtualizarNuvem(float t, float deltaTime);
    void DesmontarNuvem();
    [[nodiscard]] Vector2 LugarDoHost(int k) const;
    struct Preso {
        BossProjectile* p = nullptr;
        Vector2 velocidade;
    };
    struct Vm {
        BossProjectile* p = nullptr;
        int id = 0;
        int host = 0;
        int destino = -1;          ///< migrando para este host (-1: parada)
        float migraDesde = 0.0f;
        float proximoKernel = 0.0f;
        float proximoTrap = 0.0f;
        Vector2 onde;              ///< o centro da caixa no quadro anterior
        std::vector<Preso> presos; ///< os tiros do kernel, quicando dentro da caixa
    };
    std::vector<Vm> mVms;
    int mProximoIdDeVm = 1;
    int mVmsAntes = 0;
    float mProximaMigracao = 0.0f;
    float mReporDepois = 0.0f;
    int mMigrarAVez = 0;
    struct Trap {
        BossProjectile* p = nullptr;
        float chega = 0.0f;
    };
    std::vector<Trap> mTraps;

    /// As cores dos processos P1 a P9 - as mesmas dos cartoes (gerar_goulart.py) - e das VMs.
    static constexpr std::array<SDL_Color, 9> kCores = {{
        {255, 120, 100, 230}, {110, 200, 255, 230}, {140, 230, 140, 230}, {255, 200, 90, 230}, {200, 150, 255, 230},
        {255, 140, 200, 230}, {120, 230, 210, 230}, {230, 230, 120, 230}, {180, 180, 210, 230}}};
    static constexpr SDL_Color kEixo = {150, 170, 210, 90};
    static constexpr SDL_Color kDivisoria = {150, 170, 210, 40};
    static constexpr SDL_Color kProxima = {255, 214, 90, 150};
    static constexpr SDL_Color kPerigo = {255, 90, 80, 190};
    static constexpr SDL_Color kVoce = {130, 255, 160, 120};
    static constexpr SDL_Color kPonteiro = {250, 214, 110, 210};
    static constexpr SDL_Color kMapeamento = {130, 200, 250, 45};
    static constexpr SDL_Color kTrilha = {150, 170, 210, 120};
    static constexpr SDL_Color kBraco = {250, 214, 110, 150};
    static constexpr SDL_Color kRastro = {200, 210, 240, 70};
};
