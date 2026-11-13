//
// A chefe da INF 221 - Engenharia de Software.
//

#pragma once

#include <array>
#include <optional>
#include <string>
#include <vector>

#include <SDL_pixels.h>

#include "../Boss.h"
#include "../../../Attacks/BalaDePrata.h"
#include "../../../Attacks/Cascata.h"
#include "../../../Attacks/Decorator.h"
#include "../../../Attacks/Observer.h"
#include "../../../Attacks/Scrum.h"

class DrawLinhasComponent;

/// A professora de Engenharia de Software (INF 221), que tambem da IHC. Um processo ou padrao por
/// prova:
///   1. a cascata - a cachoeira de artefatos e a mudanca de escopo que desce a escada inteira;
///   2. o Scrum - o quadro A FAZER / FAZENDO / FEITO, as dailies e as entregas;
///   3. os padroes de projeto - o Observer (os olhos em orbita) e o Decorator (a bala que ganha
///      camadas);
///   e a repescagem: nao ha bala de prata - as promessas contra o lobisomem.
///
/// Toda prova anda pelo relogio, e nao pelo que esta em campo: o Ponto Extra limpa a tela, e o que
/// vem depois vem igual.
class MariaLucia : public Boss {
public:
    explicit MariaLucia(Scene* scene);
    ~MariaLucia() override = default;

    void OnUpdate(float deltaTime) override;
    [[nodiscard]] std::optional<Vector2> AlvoDoMovimento() const override;
    void OnDerrubou(BossProjectile* qual) override;
    void AoTerminarABatalha() override;

    /// A repescagem: nao ha bala de prata.
    static constexpr const char* kFaseDaBala = "StateFinal";

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;
    void OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                              const std::string& stateName) override;

private:
    // ---- A luta sorteada ----

    void SortearALuta();
    bool mLutaSorteada = false;
    Variacao::Sorteio mSorteio{0u};
    Observer::Sujeito mSujeitoDaLuta;
    std::vector<int> mOrdemDasPromessas;
    Cascata::Degrau mDegrauDaCascata;
    Decorator::Degrau mDegrauDoDecorator;
    BalaDePrata::Degrau mDegrauDaBala;

    // ---- O que as fases dividem ----

    std::vector<BossProjectile*> DispararEm(const char* gatilho, Vector2 onde, int quantos = 0,
                                            std::optional<float> centro = std::nullopt,
                                            std::optional<float> abertura = std::nullopt,
                                            std::optional<float> velocidade = std::nullopt,
                                            int alvo = -1);
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

    [[nodiscard]] float AnguloAte(Vector2 de, Vector2 para) const;
    [[nodiscard]] static bool Vivo(const BossProjectile* p);
    static void Matar(BossProjectile*& p);
    static void Cara(BossProjectile* p, const std::string& cara);

    std::string mFase;
    void Desmontar();

    DrawLinhasComponent* mLinhas = nullptr;
    static constexpr int kOrdemDasLinhas = 85;
    void Linha(Vector2 de, Vector2 ate, SDL_Color cor);

    void PlacarDaEtapa(const std::string& fase, size_t etapa, const std::string& texto);
    void PlacarDaFase(const std::string& fase, const std::string& texto);
    /// Um texto no placar por um instante, e depois volta ao da etapa.
    std::string mAvisoNoPlacar;
    float mAvisoAte = -1.0f;

    float mRelogio = 0.0f;
    std::optional<Vector2> mAlvo;

    // ---- Fase 1: a cascata ----

    void AtualizarCascata(float t);
    [[nodiscard]] Vector2 Caixa(int k) const;
    std::array<BossProjectile*, Cascata::kEtapas.size()> mPlacas{};
    int mEtapaDaCascata = -1;
    float mProximaQueda = 0.0f;
    float mLadoDaQueda = 1.0f;
    float mProximaMudanca = 0.0f;
    struct Mudanca {
        BossProjectile* p = nullptr;
        int caixa = -1;          ///< a caixa em que esta (-1: ainda voando ate a primeira)
        float desde = 0.0f;      ///< quando chegou nela
        Vector2 de;              ///< para a descida: de onde saiu
    };
    std::vector<Mudanca> mMudancas;

    // ---- Fase 2: o Scrum ----

    void AtualizarScrum(float t);
    void Quadro(float& esquerda, float& largura, float& topo) const;
    struct Historia {
        BossProjectile* p = nullptr;
        int pontos = 1;
        int coluna = 0;
        int ordem = 0;           ///< a ordem de chegada, para a pilha da coluna
        float proximoTrabalho = 0.0f;
    };
    std::vector<Historia> mHistorias;
    std::vector<BossProjectile*> mEntregues;
    std::array<BossProjectile*, Scrum::kColunas> mTitulos{};
    BossProjectile* mCursor = nullptr;
    Vector2 mOndeOCursor;
    float mCliqueAte = -1.0f;
    int mSprintDaVez = -2;
    int mDailiesDaVez = 0;
    int mPontosDaSprint = 0;
    std::vector<int> mFaltavam;   ///< o burndown: os pontos que faltavam no planning e em cada daily

    // ---- Fase 3: os padroes de projeto ----

    void AtualizarPadroes(float t, float deltaTime);
    struct Olho {
        BossProjectile* p = nullptr;
        float inscrito = 0.0f;
    };
    std::vector<Olho> mOlhos;
    float mProximaInscricao = 0.0f;
    std::vector<Observer::Sujeito> mSujeitos;
    float mPiscando = 0.0f;

    struct Decorada {
        BossProjectile* p = nullptr;
        int camadas = 0;
        float proxima = 0.0f;    ///< quando ganha o decorador seguinte
        bool descascada = false; ///< o aluno ja tirou uma camada: nao ganha mais
        Vector2 velocidade;
        float guiaAte = -1.0f;   ///< ate quando a Teleguiada vira para o aluno
    };
    std::vector<Decorada> mDecoradas;
    float mProximoComponente = 0.0f;
    float mLadoDoComponente = 1.0f;

    // ---- A repescagem: o lobisomem ----

    void AtualizarLobisomem(float t, float deltaTime);
    BossProjectile* mLobo = nullptr;
    float mLoboVolta = 0.0f;
    float mLoboNasceu = 0.0f;
    float mProximoUivo = 0.0f;
    float mUivandoAte = -1.0f;
    int mBalasNoLobo = 0;
    struct Bala {
        BossProjectile* p = nullptr;
        int promessa = 0;
        float guiaAte = 0.0f;    ///< ate quando ela vira para o aluno
    };
    std::vector<Bala> mBalas;
    float mProximaBala = 0.0f;
    int mProximaPromessa = 0;
    [[nodiscard]] const std::string& Promessa(int k) const;

    static constexpr SDL_Color kAgua = {120, 190, 255, 90};
    static constexpr SDL_Color kColuna = {150, 180, 220, 80};
    static constexpr SDL_Color kBurndown = {250, 214, 90, 200};
    static constexpr SDL_Color kEixo = {150, 180, 220, 120};
    static constexpr SDL_Color kAssinatura = {130, 200, 250, 70};
    static constexpr SDL_Color kNotificando = {210, 240, 255, 210};
};
