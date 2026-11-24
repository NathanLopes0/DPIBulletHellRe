//
// O chefe da INF 321 - Programacao Web.
//

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "../Boss.h"
#include "../../../Attacks/Dom.h"
#include "../../../Attacks/Fenix.h"
#include "../../../Attacks/Masmorra.h"

class BossProjectile;
class DrawLinhasComponent;
class EtiquetaDeCodigo;

/// O professor de Programacao Web (INF 321). Fase 1: a <div> que as regras de CSS mudam (Dom.h);
/// fase 2: a mesma <div>, com as regras de JavaScript, que mudam o comportamento dela; fase 3: a sala
/// de masmorra do Elixir, a corrida pelo bau - o aluno com a chave das gosmas, o professor acendendo as
/// tochas (Masmorra.h); e a final: o Phoenix, com a fenix que renasce das cinzas (Fenix.h).
class Lucas : public Boss {
public:
    explicit Lucas(Scene* scene);
    ~Lucas() override = default;

    void OnUpdate(float deltaTime) override;
    void AoTerminarABatalha() override;

    [[nodiscard]] std::optional<Vector2> AlvoDoMovimento() const override { return mAlvo; }
    /// O Elixir do aluno: enquanto dura, cada acerto vale mais (Masmorra::Degrau::fatorDoGanho).
    [[nodiscard]] float FatorDoGanho() const override;
    /// A gosma, a fenix ou as cinzas que o tiro do aluno derrubou.
    void OnDerrubou(BossProjectile* qual) override;

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;
    void OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                              const std::string& stateName) override;

private:
    // ---- A luta sorteada ----
    void SortearALuta();
    bool mLutaSorteada = false;
    Variacao::Sorteio mSorteioDoDom{0u};
    Variacao::Sorteio mSorteioDaMasmorra{0u};
    Variacao::Sorteio mSorteioDaFenix{0u};
    Dom::Degrau mDegrauDoDom;
    Masmorra::Degrau mDegrauDaMasmorra;
    Fenix::Degrau mDegrauDaFenix;

    // ---- O que as fases dividem ----

    /// Dispara 'gatilho' em 'onde' e devolve o que nasceu; 'quantos', 'centro' e 'abertura' (graus)
    /// e 'velocidade' trocam o que fases.json diz, quando dados.
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
    /// Um so, com a cara dada, andando a 'velocidade' (parado sem ela).
    BossProjectile* Um(const char* gatilho, Vector2 onde, const std::string& cara,
                       std::optional<Vector2> velocidade = std::nullopt);
    /// Uma peca que o professor conduz e que nao sai do campo sozinha.
    BossProjectile* Peca(const char* gatilho, Vector2 onde, const std::string& cara);
    /// Um leque mirado no aluno, saindo de 'de'.
    void Leque(const char* gatilho, Vector2 de, int quantos, float abertura, float velocidade);
    [[nodiscard]] float AnguloAteOAluno(Vector2 de) const;
    [[nodiscard]] static bool Vivo(const BossProjectile* p);
    static void Matar(BossProjectile*& p);
    static void MatarTodos(std::vector<BossProjectile*>& ps);
    static void Cara(BossProjectile* p, const std::string& cara);
    static void Andar(BossProjectile* p, Vector2 velocidade);
    /// Segura no lugar: a peca nao anda nem envelhece (o projetil morre aos 20 s de vida).
    static void Prender(BossProjectile* p, Vector2 onde);
    [[nodiscard]] Dom::Campo CampoPuro() const;

    /// No comeco de cada quadro, antes de qualquer disparo: o que morreu e esquecido - e a regra que
    /// morreu no aluno pesa nele.
    void Sanear();

    std::string mFase;
    float mRelogio = 0.0f;
    std::optional<Vector2> mAlvo;
    /// Desfaz o que a fase deixou no campo; 'manterADiv' guarda a <div> (da fase 1 para a 2).
    void Desmontar(bool manterADiv = false);
    /// O professor no alto, de um lado ao outro.
    void Varrer(float t, float amplitude, float periodo);

    // ---- O desenho ----
    DrawLinhasComponent* mLinhas = nullptr;
    static constexpr int kOrdemDasLinhas = 84;
    void Linha(Vector2 de, Vector2 ate, SDL_Color cor);
    void Retangulo(Vector2 centro, float largura, float altura, SDL_Color cor);
    void Preencher(Vector2 centro, float largura, float altura, SDL_Color cor);
    void Circulo(Vector2 centro, float raio, SDL_Color cor, int lados = 20);
    void Disco(Vector2 centro, float raio, SDL_Color cor);
    void Tracejada(Vector2 de, Vector2 ate, SDL_Color cor, float traco = 10.0f);
    void Coracao(Vector2 centro, float tamanho, SDL_Color cor);

    std::vector<EtiquetaDeCodigo*> mEtiquetas;
    size_t mEtiquetasUsadas = 0;
    static constexpr int kOrdemDasEtiquetas = 98;
    /// Uma etiqueta de texto neste quadro, centrada em 'onde'.
    void Etiqueta(const std::string& texto, Vector2 onde, SDL_Color tinta);
    void EsconderEtiquetas();

    // ---- Fases 1 e 2: a <div> ----

    void AtualizarDom(float t, float deltaTime, bool js);
    struct DivViva {
        Dom::Div div;
        std::vector<BossProjectile*> pecas;
        std::string ultima;          ///< a ultima regra que a mudou, mostrada um instante
        float ultimaAte = -1.0f;
    };
    struct RegraViva {
        BossProjectile* p = nullptr;
        Dom::Efeito efeito = Dom::Efeito::Largura;
        float desde = 0.0f;          ///< quando o professor a lancou: o traco dele ate ela, um instante
        int div = -1;                ///< a <div> (em mDivs) que ela persegue, ou nenhuma
    };
    std::vector<DivViva> mDivs;
    std::vector<RegraViva> mRegras;
    bool mDomComecou = false;
    float mProximoLance = 0.0f;
    float mProximaTag = 0.0f;
    int mLancesNaFase = 0;          ///< os primeiros sao a demonstracao (Dom::kLancesDeDemonstracao)
    void PosicionarPecas(DivViva& d, float agora);
    void DesenharDiv(const DivViva& d, float agora, bool js);

    // ---- Fase 3: a sala do Elixir ----

    void AtualizarMasmorra(float t, float deltaTime);
    void NovoCiclo(float t);
    enum class Desfecho { EmAberto, DoAluno, DoProfessor };
    bool mRodadaNaSala = false;     ///< ja ha uma rodada (bau, gosmas, tochas) na sala
    float mProximaRodada = 0.0f;    ///< depois do desfecho, quando comeca a seguinte
    Desfecho mDesfecho = Desfecho::EmAberto;
    Vector2 mBau;
    // As tochas que o professor acende antes do bau, na ordem dele.
    std::vector<Vector2> mTochas;
    std::vector<BossProjectile*> mTochasP;
    std::vector<bool> mAcesas;
    int mAcendendo = -1;            ///< a tocha que ele esta acendendo agora
    float mAcendeEm = 0.0f;
    BossProjectile* mBauP = nullptr;
    BossProjectile* mChaveP = nullptr;
    BossProjectile* mElixirP = nullptr;
    std::vector<BossProjectile*> mGosmas;
    std::vector<Vector2> mRumoDasGosmas;
    int mGosmasDerrubadas = 0;
    Vector2 mUltimaGosma;
    int mComAChave = -1;            ///< o aluno que pegou a chave (-1: ninguem, ou ainda no chao)
    bool mChaveCaiu = false;
    int mQuemBebeu = -1;
    float mErguidoAte = -1.0f;
    float mElixirAte = -1.0f;       ///< no relogio do chefe (mRelogio): o fim do bonus do aluno
    float mInicioDaFuria = -1.0f;
    float mProximaRajada = 0.0f;
    float mProximoAnel = 0.0f;
    float mProximoLeque = 0.0f;

    // ---- A final: o Phoenix ----

    void AtualizarFenix(float t, float deltaTime);
    void FazerRenascer(float t, bool comAnel);
    enum class EstadoDaFenix { Nenhuma, Voando, Cinzas, Apagada };
    EstadoDaFenix mEstado = EstadoDaFenix::Nenhuma;
    BossProjectile* mFenixP = nullptr;
    BossProjectile* mCinzasP = nullptr;
    bool mFenixDerrubada = false;
    bool mCinzasApagadas = false;
    Vector2 mOndeCaiu;
    float mVoo = 0.0f;
    float mMeioX = 0.0f;
    int mVersao = 0;
    float mRenasceEm = -1.0f;
    float mProximaPena = 0.0f;
    float mProximoBroadcast = 0.0f;
    float mBroadcastEm = -1.0f;
    float mProximaTagDaFinal = 0.0f;
};
