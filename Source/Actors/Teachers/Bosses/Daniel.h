//
// O chefe da INF 425 - Ciencia de Dados, a primeira optativa.
//

#pragma once

#include <array>
#include <deque>
#include <optional>
#include <string>
#include <vector>

#include <SDL_pixels.h>

#include "../Boss.h"
#include "../../../Attacks/Agrupamento.h"
#include "../../../Attacks/Chamada.h"
#include "../../../Attacks/Classificacao.h"
#include "../../../Attacks/Explicacao.h"
#include "../../../Attacks/Preparacao.h"
#include "../../../Attacks/Tipos.h"

class BossProjectile;
class DrawLinhasComponent;

/// O professor de Ciencia de Dados (INF 425, optativa). No comeco, quem joga escolhe um monstrinho
/// - fogo, agua ou planta - e o tipo dele e o seu ate o fim (Tipos.h): o tiro do seu tipo passa, o
/// do tipo que voce vence voce absorve, e o do que vence voce machuca. E voce e um dado. Cada prova
/// e uma etapa de um projeto de dados:
///   1. os dados - a coleta e a preparacao: o campo e um grafico de dispersao, e o z-score procura
///      quem esta longe da propria especie - os outliers e voce. A especie anda a cada
///      transformacao, e voce anda junto;
///   2. os atributos e os grupos - a fileira da PCA que gira, e o centroide da celula do aluno que
///      vem para cima dele;
///   3. o supervisionado - a fronteira de decisao divide o campo, e a chuva de cada lado tem a
///      classe dele;
///   e a repescagem, a explicabilidade: a caixa-preta que avisa mais cedo quem pergunta, e a cascata
///   do SHAP que errou o valor real - derrubar a ponta de uma barra tira o atributo, e levar a
///   previsao ate o valor real a explica.
/// E, no comeco de cada aula depois da primeira, o sorteio de quem explica a aula passada, com uma
/// pergunta de olhar o desenho (Chamada.h).
class Daniel : public Boss {
public:
    explicit Daniel(Scene* scene);
    ~Daniel() override = default;

    void OnUpdate(float deltaTime) override;
    [[nodiscard]] std::optional<Vector2> AlvoDoMovimento() const override;
    void OnDerrubou(BossProjectile* qual) override;
    void AoSerPerguntado() override;
    void AoTerminarABatalha() override;
    /// Enquanto a roleta gira e a pergunta esta aberta, o professor espera a resposta: nao ouve
    /// duvida (o tiro murcha nele), e so as cartas contam.
    [[nodiscard]] bool AceitaDano() const override;

protected:
    void CustomizeAttackParams(AttackParams& params, const std::string& stateName) override;
    void OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                              const std::string& stateName) override;

private:
    // ---- A luta sorteada ----

    void SortearALuta();
    bool mLutaSorteada = false;
    Variacao::Sorteio mSorteio{0u};
    Preparacao::Degrau mDegrauDaPreparacao;
    Agrupamento::Degrau mDegrauDoAgrupamento;
    Classificacao::Degrau mDegrauDaClassificacao;
    Explicacao::Degrau mDegrauDaExplicacao;

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

    /// O primeiro que nasceu, ja com a cara. Com 'velocidade', nasce andando.
    BossProjectile* Um(const char* gatilho, Vector2 onde, const std::string& cara,
                       std::optional<Vector2> velocidade = std::nullopt);

    [[nodiscard]] float AnguloAteOAluno(Vector2 de) const;
    [[nodiscard]] static bool Vivo(const BossProjectile* p);
    static void Matar(BossProjectile*& p);
    static void Cara(BossProjectile* p, const std::string& cara);
    static void Parar(BossProjectile* p);
    /// As posicoes dos alunos em campo.
    [[nodiscard]] std::vector<Vector2> Alunos() const;

    std::string mFase;
    void Desmontar();

    DrawLinhasComponent* mLinhas = nullptr;
    static constexpr int kOrdemDasLinhas = 82;
    void Linha(Vector2 de, Vector2 ate, SDL_Color cor);
    void Tracejada(Vector2 de, Vector2 ate, SDL_Color cor, float traco = 8.0f);
    void Circulo(Vector2 centro, float raio, SDL_Color cor, int lados = 16);
    void Retangulo(Vector2 de, Vector2 ate, SDL_Color cor, bool tracejado);

    void PlacarDaEtapa(const std::string& fase, size_t etapa, const std::string& texto);
    /// Um texto no placar por um instante, por cima do da etapa.
    void Avisar(const std::string& texto, float duracao = 1.4f);
    /// O placar da etapa, ou o aviso enquanto ele vale.
    void Placar(const std::string& fase, size_t etapa, const std::string& texto);
    std::string mAviso;
    float mAvisoAte = -1.0f;
    float mRelogio = 0.0f;

    std::optional<Vector2> mAlvo;
    /// O professor varre o alto devagar: 'periodo' segundos de um lado ao outro e de volta.
    void Varrer(float t, float amplitude, float periodo);

    /// Um movimento de A ate B em 'dura' segundos, a partir de 'desde' (desde < 0: parado).
    struct Ida {
        Vector2 de;
        Vector2 para;
        float desde = -1.0f;
        float dura = 0.0f;
    };
    /// Onde a ida esta em 't' (suave no comeco e no fim); e se ja chegou.
    static Vector2 OndeNaIda(const Ida& ida, float t);
    static bool Chegou(const Ida& ida, float t) { return ida.desde < 0.0f || t >= ida.desde + ida.dura; }

    // ---- O tipo do aluno (Tipos.h): escolhido no comeco da fase 1, para a luta inteira ----

    /// Os monstrinhos no chao, e o toque que escolhe; no fim do tempo, quem nao escolheu fica com o
    /// mais perto.
    void Escolher(float t);
    /// Todo quadro, depois da escolha: a classe de cada aluno, o selo, o anel na cor do tipo, e a
    /// absorcao do tiro do tipo que ele vence.
    void AtualizarTipos();
    [[nodiscard]] std::vector<Vector2> LugaresDaEscolha() const;
    static constexpr const char* kGatilhoDoSelo = "selo";
    std::vector<int> mTipos;                                  ///< o de cada aluno (-1: ainda nao)
    std::array<BossProjectile*, Tipos::kTipos> mEscolhas{};
    std::vector<BossProjectile*> mSelos;
    bool mEscolheu = false;
    float mPremioDoTipo = 0.0f;
    void Elipse(Vector2 centro, Vector2 raios, SDL_Color cor);

    // ---- O sorteio (Chamada.h): o comeco das fases 2, 3 e da repescagem ----

    /// A roleta, a pergunta e a garoa (o gatilho "garoa" da fase, com a cara 'caraDaGaroa').
    void AtualizarSorteio(float t, int aula, const std::string& fase, const char* caraDaGaroa);
    static constexpr const char* kGatilhoDaGaroa = "garoa";
    void DesmontarSorteio();
    void Responder(int carta);
    [[nodiscard]] std::string NomeDe(int quem) const;
    struct Sorteado {
        bool feito = false;
        int quem = -2;          ///< -1: um colega; 0 ou 1: o aluno
        int aula = 0;
        Chamada::Questao questao;
        int colega = 0;
        std::array<BossProjectile*, Chamada::kOpcoes> cartas{};
        bool perguntou = false;
        bool respondido = false;
        std::string placar;
    };
    Sorteado mSorteado;
    float mProximaGaroa = 0.0f;
    /// O quadro do desenho da pergunta, no alto do campo; 'q' vai de 0 a 1 nele.
    [[nodiscard]] SDL_FRect QuadroDaQuestao() const;
    [[nodiscard]] Vector2 NoQuadro(Vector2 q) const;
    /// Onde a carta 'k' do sorteado fica: em cima do lugar dela, ou acima, presa pelo alfinete.
    [[nodiscard]] Vector2 OndeDaCarta(int k) const;
    static constexpr float kAlturaDoAlfinete = 44.0f;
    void DesenharQuestao();

    // ---- Fase 1: a preparacao ----

    void AtualizarPreparacao(float t, float deltaTime);
    void DesmontarPreparacao();
    [[nodiscard]] SDL_FRect Grafico() const;
    [[nodiscard]] Vector2 NoCampo(Vector2 noGrafico) const;
    [[nodiscard]] Vector2 NoGrafico(Vector2 noCampo) const;
    [[nodiscard]] Vector2 LugarDaFonte(int k) const;
    struct DadoNoGrafico {
        BossProjectile* p = nullptr;
        Preparacao::Dado dado;
        float chega = 0.0f;       ///< quando chega ao lugar dele, voando da fonte
        bool chegou = false;
        Ida ida;                  ///< a transformacao (ou a imputacao) que o move
        bool marcado = false;     ///< o z-score achou: vai estourar
    };
    std::vector<DadoNoGrafico> mDados;
    std::array<BossProjectile*, Preparacao::kFontes.size()> mFontes{};
    std::array<Vector2, Preparacao::kEspecies> mEspecies{};
    bool mEspeciesSorteadas = false;
    float mProximaColeta = 0.0f;
    int mColetados = 0;
    size_t mProximoPasso = 0;
    struct Transformando {
        Preparacao::Transformacao qual = Preparacao::Transformacao::MinMax;
        float avisoDesde = 0.0f;
        float comeca = 0.0f;
        Preparacao::Caixa caixa{};
    };
    std::optional<Transformando> mTransformando;
    bool mOutliersMarcados = false;
    bool mEstourou = false;
    bool mImputou = false;
    Vector2 mMediaDaImputacao;
    /// O aluno e um dado: a proxima varredura do z-score (Preparacao::kVarreduras).
    size_t mProximaVarredura = 0;
    int mVarreduraAvisada = -1;
    void VarrerOsAlunos(float t);

    // ---- Fase 2: os atributos e os grupos ----

    void AtualizarAgrupamento(float t, float deltaTime);
    void DesmontarAgrupamento();
    struct PontoDaNuvem {
        BossProjectile* p = nullptr;
        Vector2 projetado;        ///< onde ele cai na componente principal
        Ida ida;
    };
    std::vector<PontoDaNuvem> mNuvem;
    bool mNuvemCriada = false;
    bool mProjetou = false;
    bool mGirou = false;
    bool mAgrupou = false;
    Agrupamento::Eixo mEixo;
    float mSentidoDoGiro = 1.0f;
    std::vector<BossProjectile*> mCentroides;
    std::vector<Vector2> mCentros;
    std::vector<Ida> mIdasDosCentros;
    std::vector<bool> mAnelPendente;
    float mProximaIteracao = 0.0f;
    int mIteracao = 0;
    bool mCotovelo = false;
    float mInercia = 0.0f;
    float mInerciaInicial = 0.0f;   ///< a da primeira iteracao: o placar mostra o quanto ela caiu

    // ---- Fase 3: a classificacao ----

    void AtualizarClassificacao(float t, float deltaTime);
    void DesmontarClassificacao();
    [[nodiscard]] Vector2 Normalizado(Vector2 noCampo) const;
    [[nodiscard]] Vector2 DoNormalizado(Vector2 n) const;
    /// O tipo (fogo, agua ou planta) de cada classe da fronteira: o que vence o do aluno e o que ele vence.
    [[nodiscard]] int TipoDaClasse(int classe) const;
    [[nodiscard]] int ClasseEm(Vector2 noCampo, int etapa) const;
    bool mPerceptronComecou = false;
    Classificacao::Reta mModelo;
    Classificacao::Reta mVerdade;
    float mAnguloDaVerdade = 0.0f;
    std::optional<Classificacao::Reta> mProximoModelo;
    int mErrosDaEpoca = 0;
    float mProximaEpoca = 0.0f;
    int mEpoca = 0;
    bool mComiteComecou = false;
    Vector2 mCortes;
    std::vector<Classificacao::Arvore> mComite;
    std::array<BossProjectile*, Classificacao::kArvores> mArvores{};
    std::optional<Classificacao::Arvore> mArvoreNova;
    int mProximaArvore = 0;
    float mProximoReplantio = 0.0f;
    int mClasseDaRajada = -1;
    float mProximaRajada = 0.0f;
    float mProximaChuva = 0.0f;
    void LimparClasses();

    // ---- A repescagem: a explicacao ----

    void AtualizarExplicacao(float t, float deltaTime);
    void DesmontarExplicacao();
    BossProjectile* mCaixaPreta = nullptr;
    std::deque<float> mPerguntas;   ///< quando cada pergunta acertou o professor (mRelogio)
    [[nodiscard]] int PerguntasRecentes() const;
    struct PalpiteDaVez {
        Explicacao::Palpite tipo = Explicacao::Palpite::Anel;
        float sai = 0.0f;
        bool avisado = false;
        std::vector<float> rumos;
    };
    std::optional<PalpiteDaVez> mPalpite;
    float mProximoPalpite = 0.0f;
    float mAntecedenciaDaVez = 0.0f;
    struct BarraEmCampo {
        Explicacao::Barra barra;
        float y = 0.0f;
        float aparece = 0.0f;
        std::vector<BossProjectile*> tiros;   ///< a faixa: duas fileiras de tiros (o do aluno atravessa)
        BossProjectile* cabeca = nullptr;     ///< a ponta: a unica que o tiro do aluno derruba
        bool tirada = false;                  ///< o aluno a derrubou: o atributo saiu da previsao
    };
    std::vector<BarraEmCampo> mBarras;
    std::vector<float> mContribuicoes;
    float mReal = 0.0f;   ///< o valor real (x no campo): onde a cascata acaba sem os atributos errados
    float mProximaCascata = 0.0f;
    float mPrevisaoEm = -1.0f;

    /// As cores das especies (e dos grupos e das classes) - as mesmas de gerar_daniel.py.
    static constexpr std::array<SDL_Color, 5> kCores = {{{255, 130, 70, 230}, {90, 170, 255, 230},
                                                         {110, 210, 110, 230}, {250, 220, 70, 230},
                                                         {190, 150, 110, 230}}};
    static constexpr std::array<SDL_Color, 3> kCoresDasArvores = {{{150, 230, 150, 200}, {240, 200, 120, 200},
                                                                  {170, 190, 255, 200}}};
    static constexpr SDL_Color kEixo = {150, 170, 210, 110};
    static constexpr SDL_Color kSeta = {255, 214, 90, 150};
    static constexpr SDL_Color kPerigo = {255, 90, 80, 200};
    static constexpr SDL_Color kBase = {230, 230, 240, 120};
    static constexpr SDL_Color kFronteira = {245, 245, 255, 200};
};
