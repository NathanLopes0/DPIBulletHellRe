//
// O chefe da INF 425 - Ciencia de Dados, a primeira optativa.
//

#pragma once

#include <array>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include <SDL_pixels.h>

#include "../Boss.h"
#include "../../../Attacks/Agrupamento.h"
#include "../../../Attacks/Ecossistema.h"
#include "../../../Attacks/Sobreajuste.h"
#include "../../../Attacks/Confusao.h"
#include "../../../Attacks/Tipos.h"

class BossProjectile;
class DrawLinhasComponent;
class EtiquetaDeCodigo;
class PlayerProjectile;

/// O professor de Ciencia de Dados (INF 425, optativa). No comeco, quem joga escolhe um monstrinho
/// - fogo, agua ou planta - e o tipo dele e o seu ate o fim (Tipos.h), na defesa e no ataque. O
/// campo e um laboratorio de dados, um grafico de dispersao (x: o ataque, y: a defesa), e cada prova
/// e um passo do projeto:
///   1. a coleta (Ecossistema.h) - os dados chegam das fontes como monstrinhos e pousam no grafico,
///      em ondas que ensinam o triangulo: absorver, atravessar, desviar, e tudo junto;
///   2. as k-medias (Agrupamento.h) - os pontos da coleta viram tres grupos, cada um ataca com o tipo
///      da maioria, e os outliers puxam os centroides;
///   3. o overfitting (Sobreajuste.h) - o modelo do professor decora os passos do aluno e atira onde
///      ele esteve e para onde ia; na validacao ele nao generaliza, a curva se desfaz, e os tres
///      nucleos ficam abertos;
///   e a repescagem, a matriz de confusao (Confusao.h) - o modelo classifica dados novos, e cada erro
///      dele, fora da diagonal, ataca ate o aluno corrigi-lo.
/// Os efeitinhos de cada elemento (Tipos::Particula) contam o que aconteceu sem uma palavra.
class Daniel : public Boss {
public:
    explicit Daniel(Scene* scene);
    ~Daniel() override = default;

    void OnUpdate(float deltaTime) override;
    [[nodiscard]] std::optional<Vector2> AlvoDoMovimento() const override { return mAlvo; }
    void AoTerminarABatalha() override;

protected:

private:
    // ---- A luta sorteada ----

    void SortearALuta();
    bool mLutaSorteada = false;
    Variacao::Sorteio mSorteio{0u};
    Variacao::Sorteio mSorteioDosEfeitos{0u};
    Ecossistema::Degrau mDegrauDaColeta;
    Agrupamento::Degrau mDegrauDoAgrupamento;
    Sobreajuste::Degrau mDegrauDoSobreajuste;
    Confusao::Degrau mDegrauDaConfusao;

    // ---- O que as fases dividem ----


    /// O primeiro que nasceu, ja com a cara. Com 'velocidade', nasce andando.
    BossProjectile* Um(const char* gatilho, Vector2 onde, const std::string& cara,
                       std::optional<Vector2> velocidade = std::nullopt);
    /// Um monstrinho do 'tipo': a cara e a classe dele (a do aluno do mesmo tipo passa).
    BossProjectile* Monstrinho(const char* gatilho, Vector2 onde, int tipo, std::optional<Vector2> velocidade);
    /// Um leque do professor, mirado no aluno.
    void Leque(Vector2 de, int quantos, float abertura, float velocidade);

    [[nodiscard]] float AnguloAteOAluno(Vector2 de) const;

    std::string mFase;
    float mRelogio = 0.0f;
    /// 'manterOsPontos': da coleta para as k-medias, os pontos do grafico ficam.
    void Desmontar(bool manterOsPontos = false);
    std::optional<Vector2> mAlvo;
    /// O professor varre o alto devagar: 'periodo' segundos de um lado ao outro e de volta.
    void Varrer(float t, float amplitude, float periodo);

    // O desenho: o das linhas (Boss::mLinhas) embaixo dos projeteis, e o dos efeitos por cima.
    DrawLinhasComponent* mEfeitos = nullptr;
    static constexpr int kOrdemDasLinhas = 82;
    static constexpr int kOrdemDosEfeitos = 99;
    void Linha(Vector2 de, Vector2 ate, SDL_Color cor, bool emCima = false);
    void Circulo(Vector2 centro, float raio, SDL_Color cor, int lados = 16, bool emCima = false);
    void Disco(Vector2 centro, float raio, SDL_Color cor, bool emCima = false);
    std::vector<EtiquetaDeCodigo*> mEtiquetas;

    // ---- O grafico: o campo do laboratorio ----

    [[nodiscard]] SDL_FRect Grafico() const;
    /// De 0 a 1 do grafico (y = 1 em cima) para o campo.
    [[nodiscard]] Vector2 NoCampo(Vector2 noGrafico) const;
    void DesenharGrafico();
    /// O triangulo no canto: os tres tipos e quem vence quem, com o do aluno marcado.
    void DesenharTriangulo();
    std::array<BossProjectile*, Tipos::kTipos> mLegenda{};

    // ---- O tipo do aluno: escolhido no comeco da fase 1, para a luta inteira ----

    void Escolher(float t, float deltaTime);
    void AtualizarTipos();
    [[nodiscard]] std::vector<Vector2> LugaresDaEscolha() const;
    /// O tipo de quem atirou (-1: nao e um aluno, ou nao escolheu).
    [[nodiscard]] int TipoDoTiro(const PlayerProjectile* tiro) const;
    static constexpr const char* kGatilhoDoSelo = "selo";
    std::vector<int> mTipos;                                  ///< o de cada aluno (-1: ainda nao)
    std::array<BossProjectile*, Tipos::kTipos> mEscolhas{};
    std::array<int, Tipos::kTipos> mLugarDoTipo{0, 1, 2};
    std::vector<float> mEmCima;
    std::vector<int> mEmCimaDe;
    std::vector<BossProjectile*> mSelos;
    bool mEscolheu = false;
    float mPremioDoTipo = 0.0f;
    /// Os tiros do tipo do aluno que ja passaram por ele (o efeitinho sai uma vez so).
    std::set<const BossProjectile*> mJaPassou;

    // ---- Os efeitinhos ----

    std::vector<Tipos::Particula> mParticulas;
    void Efeito(int tipo, Vector2 onde, int quantas, float forca);
    void AndarOsEfeitos(float deltaTime);
    /// O rastro de cada tiro do aluno, no elemento dele.
    void RastroDosTiros();
    /// O tiro do aluno que chega a 'raio' de 'onde' acerta o alvo do 'tipo': o tiro some, e a volta
    /// e o dano (com o fator do triangulo). 'fechado': o alvo nao apanha, so faz "tim".
    float Golpear(Vector2 onde, float raio, int tipo, bool fechado = false);

    // ---- Fase 1: a coleta ----

    void AtualizarColeta(float t, float deltaTime);

    // ---- Final: a matriz de confusao ----
    /// Um monstrinho classificado: voa ate a casa que o modelo escolheu; na diagonal fica quieto, fora
    /// dela e um erro, que atira ate ser corrigido (desliza para a diagonal) ou vazar.
    struct Classificado {
        BossProjectile* p = nullptr;
        int real = 0;
        int previsto = 0;
        bool pousou = false;
        float vida = 0.0f;
        float vaza = 0.0f;          ///< quando o erro vaza (no relogio da fase)
        float proximoTiro = 0.0f;
        float corrigidoEm = -1.0f;  ///< quando comecou a deslizar para a diagonal (-1: nao)
        Vector2 de;                 ///< de onde ele desliza
    };
    std::vector<Classificado> mClassificados;
    Confusao::Matriz mMatriz{};
    float mProximaClassificada = 0.0f;
    [[nodiscard]] Confusao::Grade GradeDaMatriz() const;
    void AtualizarConfusao(float t, float deltaTime);
    struct Amostra {
        BossProjectile* p = nullptr;
        int tipo = 0;
        float vida = Ecossistema::kVidaDaAmostra;
        Vector2 destino;
    };
    std::vector<Amostra> mAmostras;          ///< as que ainda voam
    struct Ponto {
        BossProjectile* p = nullptr;
        int tipo = -1;                       ///< -1: outlier
        Vector2 onde;
    };
    std::vector<Ponto> mPontos;              ///< as que pousaram: o conjunto de dados
    std::array<BossProjectile*, Ecossistema::kFontes.size()> mFontes{};
    float mProximaAmostra = 0.0f;
    float mProximoLeque = 0.0f;
    int mAmostrasFeitas = 0;
    void Pousar(Amostra& a);

    // ---- Fase 2: as k-medias ----

    void AtualizarAgrupamento(float t, float deltaTime);
    bool mAgrupou = false;
    std::vector<BossProjectile*> mCentroidesP;
    std::vector<Vector2> mCentros;
    std::vector<Vector2> mDeOnde;
    std::vector<Vector2> mParaOnde;
    std::vector<int> mGrupos;
    std::vector<int> mTiposDosGrupos;
    float mDeslizeDesde = -1.0f;
    float mAtribuiuEm = -10.0f;
    float mProximaIteracao = 0.0f;
    struct Outlier {
        BossProjectile* p = nullptr;
        float vida = Agrupamento::kVidaDoOutlier;
        Vector2 destino;
        bool pousou = false;
    };
    std::vector<Outlier> mOutliers;
    float mProximoOutlier = 0.0f;
    int mOutliersFeitos = 0;

    // ---- Fase 3: o overfitting ----

    void AtualizarSobreajuste(float t, float deltaTime);
    bool mModeloComecou = false;
    std::vector<Sobreajuste::Memoria> mMemorias;
    struct Nucleo {
        BossProjectile* p = nullptr;
        float vida = 0.0f;
        bool quebrado = false;
    };
    std::array<Nucleo, Sobreajuste::kNucleos> mNucleos{};
    float mProximoTiro = 0.0f;
    int mAlunoDaVez = 0;
    float mProximoDecorado = 0.0f;
    float mDecoradoEm = -1.0f;               ///< quando os tiros no lugar decorado saem (-1: nenhum)
    Vector2 mDecorado;
    /// O modelo riscado: os pedacos da curva, que caem.
    std::vector<std::pair<Vector2, Vector2>> mPedacos;
    bool mRiscou = false;
    /// O veredito da validacao, acima da curva: a culpa e do modelo, e nao do aluno.
    EtiquetaDeCodigo* mVeredito = nullptr;
    /// A fase 3 explicada no campo: cada rajada da previsao deixa um fantasma onde o modelo acha que o
    /// aluno vai estar, e quando ela chega e o aluno nao esta la, "errou!".
    struct Fantasma {
        Vector2 onde;
        size_t aluno = 0;
        float saiu = 0.0f;      ///< quando a rajada saiu (no relogio da fase)
        float chega = 0.0f;     ///< quando ela chega la
        bool julgado = false;
        bool errou = false;
    };
    std::vector<Fantasma> mFantasmas;
    /// So as primeiras rajadas dizem "previsão"; e um "errou!" de cada vez, para nao encher a tela.
    int mRajadasDaPrevisao = 0;
    float mUltimoErrou = -10.0f;
    static constexpr int kRajadasComNome = 3;
    static constexpr float kEntreErrous = 1.2f;
    /// Os rotulos curtos do campo ("previsão", "errou!", "decorado"), reaproveitados a cada quadro.
    std::vector<EtiquetaDeCodigo*> mRotulos;
    size_t mRotulosUsados = 0;
    void Rotulo(const std::string& texto, Vector2 onde, SDL_Color tinta, int largura = 96);
    void EsconderRotulosSobrando();
};
