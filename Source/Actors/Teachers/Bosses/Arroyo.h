//
// O chefe da INF 332 - Projeto e Analise de Algoritmos.
//

#pragma once

#include <array>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <SDL_pixels.h>

#include "../Boss.h"
#include "../../../Attacks/Intratavel.h"
#include "../../../Attacks/Paradigmas.h"
#include "../../../Attacks/Recorrencia.h"
#include "../../../Attacks/Tempera.h"

class BossProjectile;
class DrawLinhasComponent;

/// O professor de Projeto e Analise de Algoritmos (INF 332). Cada prova e um pedaco da ementa:
///   1. a analise - a arvore de recursao de T(n) = aT(n/b) + f(n), e o Teorema Mestre no placar;
///   2. os paradigmas - o troco guloso, a tabela da programacao dinamica e as seis rainhas;
///   3. os limites - a rede de ordenacao (o limite inferior) e o caixeiro viajante (a NP-completude);
///   e a repescagem, a aproximacao: a tempera simulada.
///
/// Toda prova se refaz sozinha: o que o Ponto Extra limpa volta na rodada seguinte, porque o que
/// anda e o algoritmo, e nao os projeteis.
class Arroyo : public Boss {
public:
    explicit Arroyo(Scene* scene);
    ~Arroyo() override = default;

    void OnUpdate(float deltaTime) override;
    [[nodiscard]] std::optional<Vector2> AlvoDoMovimento() const override;
    void AoTerminarABatalha() override;

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;
    void OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                              const std::string& stateName) override;

private:
    // ---- A luta sorteada ----

    void SortearALuta();
    bool mLutaSorteada = false;
    std::vector<int> mOrdemDasRecorrencias;
    Variacao::Sorteio mSorteio{0u};

    Recorrencia::Degrau mDegrauDaArvore;
    Paradigmas::Degrau mDegrauDosParadigmas;
    Intratavel::Degrau mDegrauDoIntratavel;
    Tempera::Degrau mDegrauDaTempera;

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

    /// O primeiro que nasceu, ja com a cara e a escala; nullptr se nao nasceu nada. Com 'velocidade',
    /// nasce andando: o WaveAttack poe a velocidade do disparo no primeiro quadro do projetil
    /// (ActivateBehavior), e uma velocidade dada antes disso se perderia.
    BossProjectile* Um(const char* gatilho, Vector2 onde, const std::string& cara, float escala = 1.0f,
                       std::optional<Vector2> velocidade = std::nullopt);

    [[nodiscard]] float AnguloAteOAluno(Vector2 de) const;
    [[nodiscard]] static bool Vivo(const BossProjectile* p);
    static void Matar(BossProjectile*& p);

    /// A fase em que o professor estava no quadro anterior: a troca desmonta a anterior.
    std::string mFase;
    void Desmontar();

    DrawLinhasComponent* mLinhas = nullptr;
    static constexpr int kOrdemDasLinhas = 82;
    void Linha(Vector2 de, Vector2 ate, SDL_Color cor);

    /// O placar de uma etapa da fase, so quando muda.
    void PlacarDaEtapa(const std::string& fase, size_t etapa, const std::string& texto);
    void PlacarDaFase(const std::string& fase, const std::string& texto);

    std::optional<Vector2> mAlvo;

    // ---- Fase 1: a arvore de recursao ----

    void AtualizarArvore(float t);
    struct No {
        BossProjectile* p = nullptr;
        int nivel = 0;
        float nasceu = 0.0f;
        Vector2 rumo;          ///< a velocidade, que o no guarda ate se dividir
        Vector2 origem;        ///< onde o pai se dividiu: a aresta da arvore
    };
    std::vector<No> mNos;
    int mRodada = -1;
    bool mRaizSaiu = false;
    bool mArvorePronta = false;

    // ---- Fase 2: os paradigmas ----

    void AtualizarParadigmas(float t);
    void DesmontarParadigmas();
    int mEtapaDosParadigmas = -1;

    // O guloso.
    int mTrocoDaVez = -1;
    std::vector<int> mMoedas;
    size_t mMoedaDaVez = 0;
    float mProximaMoeda = 0.0f;

    // A programacao dinamica.
    std::vector<std::vector<int>> mTabela;
    std::vector<std::vector<std::pair<int, int>>> mOndas;
    size_t mOndaDaVez = 0;
    float mProximaOnda = 0.0f;
    std::vector<BossProjectile*> mCasas;
    float mXDaOnda = 0.0f;
    bool mTabelaPronta = false;

    // O backtracking.
    std::vector<Paradigmas::Passo> mBusca;
    size_t mPassoDaVez = 0;
    float mProximoPasso = 0.0f;
    std::array<BossProjectile*, Paradigmas::kRainhas> mRainhas{};
    int mUltimaColuna = 0;
    bool mSolucaoAtirou = false;

    [[nodiscard]] Vector2 CentroDoTabuleiro() const;
    [[nodiscard]] float TopoDaTabela() const;

    // ---- Fase 3: o limite inferior e o caixeiro ----

    void AtualizarIntratavel(float t);
    void DesmontarIntratavel();

    struct Leva {
        std::array<BossProjectile*, Intratavel::kFios> porFio{};
        std::array<int, Intratavel::kFios> valores{};
        int camada = 0;
        float trocandoAte = -1.0f;
    };
    std::vector<Leva> mLevas;
    float mProximaLeva = 0.0f;
    [[nodiscard]] float XDoFio(int i) const;
    [[nodiscard]] float YDaCamada(int c) const;

    int mRodadaDoCaixeiro = -1;
    std::vector<Vector2> mOndeCidades;
    std::vector<BossProjectile*> mCidades;
    std::vector<int> mRota;
    std::vector<int> mMelhor;
    float mMelhorComprimento = 0.0f;
    int mTentativa = 0;
    float mInicioDaVolta = 0.0f;
    size_t mProximaParada = 0;
    BossProjectile* mCaixeiro = nullptr;
    void ComecarVolta(float t);

    // ---- A repescagem: a tempera simulada ----

    void AtualizarTempera(float t, float deltaTime);
    void DesmontarTempera();
    struct Solucao {
        BossProjectile* p = nullptr;
        Vector2 alvo;          ///< o ponto aceito, para onde ela vai
        float proxima = 0.0f;  ///< quando propoe o passo seguinte
    };
    std::vector<Solucao> mSolucoes;
    std::vector<Vector2> mOndeVales;
    std::vector<BossProjectile*> mVales;
    float mProximaSolucao = 0.0f;
    float mUltimoPlacar = -1.0f;
    int mMetadeCongelada = -1;

    static constexpr SDL_Color kAresta = {150, 220, 200, 120};
    static constexpr SDL_Color kAtaqueDaRainha = {255, 120, 150, 70};
    static constexpr SDL_Color kTabuleiro = {200, 200, 230, 45};
    static constexpr SDL_Color kFio = {140, 170, 220, 70};
    static constexpr SDL_Color kComparador = {250, 214, 110, 150};
    /// Os segundos de uma ida e volta do professor por cima do mapa do caixeiro.
    static constexpr float kVaiVemDoCaixeiro = 8.0f;
    static constexpr SDL_Color kMelhorRota = {250, 214, 110, 170};
    static constexpr SDL_Color kRotaDaVez = {230, 240, 255, 60};
};
