//
// O chefe da INF 429 - Visao Computacional, a segunda optativa.
//

#pragma once

#include <array>
#include <optional>
#include <string>
#include <vector>

#include <SDL_pixels.h>

#include "../Boss.h"
#include "../../../Attacks/CameraEscura.h"
#include "../../../Attacks/Convolucao.h"
#include "../../../Attacks/Laboratorio.h"
#include "../../../Attacks/RedeNeural.h"

class BossProjectile;
class DrawLinhasComponent;

/// O professor de visao computacional (INF 429, optativa), que lidera o MaVIlab. Cada prova pede que
/// quem joga pense em como a maquina ve, e nao so desvie:
///   1. a imagem - a camera escura: o perigo e a imagem invertida do que ele arremessa, e ela vem de
///      baixo; o foco muda o aumento, e o desfoque faz anel de tudo o que nao esta na sua altura;
///   2. os descritores - as setas do gradiente levam os tiros (e a imagem gira, e elas nao); na
///      convolucao, os tiros do aluno acendem a imagem que o filtro le; no pooling, tudo vira vetor;
///   3. a rede - cada neuronio que dispara manda pulsos (trens de tiros) pelas ligacoes fortes,
///      cruzando o campo; derrubar um neuronio e o dropout, e corta os pulsos das ligacoes dele; a
///      retropropagacao aprende onde o aluno esta; os atalhos da residual pulam o dropout;
///   e a repescagem, no MaVIlab: a mascara do encoder-decoder, a recorrente que preve o caminho, e o
///   detector que procura o aluno entre os colegas do laboratorio.
class Michel : public Boss {
public:
    explicit Michel(Scene* scene);
    ~Michel() override = default;

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
    CameraEscura::Degrau mDegrauDaCamera;
    Convolucao::Degrau mDegrauDaConvolucao;
    RedeNeural::Degrau mDegrauDaRede;
    Laboratorio::Degrau mDegrauDoLaboratorio;

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
    [[nodiscard]] static float AnguloAte(Vector2 de, Vector2 para);
    [[nodiscard]] static bool Vivo(const BossProjectile* p);
    static void Matar(BossProjectile*& p);
    static void Cara(BossProjectile* p, const std::string& cara);
    static void Parar(BossProjectile* p);
    static void Mover(BossProjectile* p, Vector2 onde, Vector2 velocidade);
    /// As posicoes dos alunos em campo.
    [[nodiscard]] std::vector<Vector2> Alunos() const;
    [[nodiscard]] bool NoCampo(Vector2 p, float folga = 0.0f) const;

    std::string mFase;
    void Desmontar();

    DrawLinhasComponent* mLinhas = nullptr;
    static constexpr int kOrdemDasLinhas = 82;
    void Linha(Vector2 de, Vector2 ate, SDL_Color cor);
    void Tracejada(Vector2 de, Vector2 ate, SDL_Color cor, float traco = 8.0f);
    void Circulo(Vector2 centro, float raio, SDL_Color cor, int lados = 16);
    void Retangulo(Vector2 de, Vector2 ate, SDL_Color cor, bool tracejado = false);
    void Preencher(Vector2 de, Vector2 ate, SDL_Color cor);

    void PlacarDaEtapa(const std::string& fase, size_t etapa, const std::string& texto);
    /// Um texto no placar por um instante, por cima do da etapa.
    void Avisar(const std::string& texto, float duracao = 1.6f);
    /// O placar da etapa, ou o aviso enquanto ele vale.
    void Placar(const std::string& fase, size_t etapa, const std::string& texto);
    std::string mAviso;
    float mAvisoAte = -1.0f;
    float mRelogio = 0.0f;
    int mEtapaAnterior = -1;

    std::optional<Vector2> mAlvo;
    /// O professor varre o alto: 'periodo' segundos de um lado ao outro e de volta.
    void Varrer(float t, float amplitude, float periodo);

    // ---- Fase 1: a camera escura ----

    void AtualizarCamera(float t, float deltaTime);
    void DesmontarCamera();
    struct ObjetoNaCena {
        BossProjectile* objeto = nullptr;
        std::vector<BossProjectile*> imagens;   ///< uma (nitida) ou o anel do borrao
        int tipo = 0;
        float giro = 0.0f;
    };
    std::vector<ObjetoNaCena> mCena;
    BossProjectile* mCamera = nullptr;
    BossProjectile* mReflexo = nullptr;
    Vector2 mFuro;
    float mAumento = 1.0f;
    float mFoco = 0.0f;
    bool mFocoComecou = false;
    float mProximoArremesso = 0.0f;
    int mArremessos = 0;
    /// As imagens de um objeto: nascem quando entram no campo e somem quando saem dele.
    void AtualizarImagens(ObjetoNaCena& o, int etapa, float deltaTime);

    // ---- Fase 2: os descritores e a convolucao ----

    void AtualizarConvolucao(float t, float deltaTime);
    void DesmontarConvolucao();
    [[nodiscard]] Vector2 CentroDaCelula(int c, int l) const;
    [[nodiscard]] std::pair<int, int> CelulaEm(Vector2 p) const;
    [[nodiscard]] Vector2 TamanhoDaCelula() const;
    Convolucao::Grade mBrilho;
    Convolucao::Grade mAtivacao;   ///< o mapa de ativacao, so para ver (apaga sozinho)
    std::vector<int> mBins;
    int mGiro = 0;
    float mProximaAtualizacao = 0.0f;
    struct Fluxo {
        BossProjectile* p = nullptr;
        float morre = 0.0f;
    };
    std::vector<Fluxo> mFluxos;
    float mProximoFluxo = 0.0f;
    int mPassoDaJanela = -1;
    float mProximoPasso = 0.0f;
    int mPassoDoPooling = -1;
    float mProximoPooling = 0.0f;
    float mProximoLeque = 0.0f;
    bool mVetorDisparado = false;
    float mUltimaResposta = 0.0f;
    // O filtro de borda (o Dificil): a janela que vem de tras para a frente.
    int mPassoDaBorda = -1;
    float mProximaBorda = 0.0f;
    float mUltimaBorda = 0.0f;
    /// O anel do segundo mapa, logo depois do primeiro (o Dificil); -1: nao ha, ou ja saiu.
    float mSegundoVetor = -1.0f;
    int mTirosDoVetor = 0;
    static constexpr float kEntreOsVetores = 0.28f;
    /// Uma ativacao que ja passou do limiar e pisca antes de disparar: de onde, quantos tiros, a
    /// abertura, quando, e as celulas que ela acende.
    struct Ativacao {
        Vector2 de;
        int tiros = 0;
        float abertura = 0.0f;
        float velocidade = 0.0f;
        float quando = 0.0f;
        int coluna = 0;
        int linha = 0;
        int lado = 1;            ///< 1: a celula; 2: o bloco 2x2 do pooling
        float valor = 0.0f;
        bool borda = false;      ///< do filtro de borda: pisca na cor dele
    };
    std::vector<Ativacao> mPendentes;

    // ---- Fase 3: a rede ----

    void AtualizarRede(float t, float deltaTime);
    void DesmontarRede();
    void MontarARede(int etapa);
    [[nodiscard]] Vector2 LugarDoNeuronio(int camada, int i) const;
    void Disparou(int camada, int i);
    /// Um vagao de um pulso: de 'de' (o neuronio 'iDe' da camada 'camadaDe'; -1, o professor) ao
    /// 'ate' da 'camada'. So o primeiro vagao ('leva') soma o peso no neuronio de chegada.
    void MandarSinal(Vector2 de, int camada, int ate, float peso, bool atalho, int camadaDe = -1, int iDe = -1,
                     bool leva = true);
    /// O dropout do neuronio 'i' da camada 'k': os pulsos das ligacoes dele, no caminho ou por sair,
    /// se perdem.
    void Cortar(int k, int i);
    /// Se o neuronio esta fora da rede agora.
    [[nodiscard]] bool Fora(int k, int i) const;
    RedeNeural::Rede mRede;
    int mEtapaDaRede = -1;
    struct Neuronio {
        BossProjectile* p = nullptr;
        float soma = 0.0f;
        float foraAte = -1.0f;   ///< o dropout: mRelogio em que ele volta
        float acesoAte = -1.0f;
    };
    std::vector<std::vector<Neuronio>> mNeuronios;
    struct Sinal {
        BossProjectile* p = nullptr;
        int camada = 0;          ///< a camada de chegada
        int ate = 0;
        float peso = 0.0f;
        Vector2 de;
        Vector2 para;
        float sai = 0.0f;
        float chega = 0.0f;
        bool atalho = false;
        int camadaDe = -1;       ///< o neuronio de onde ele saiu (-1: o professor)
        int de_ = -1;
        bool leva = true;        ///< o primeiro vagao: e ele que soma no neuronio de chegada
    };
    std::vector<Sinal> mSinais;
    /// Os vagoes que ainda vao sair: a ligacao fica acesa enquanto isso - o aviso.
    struct PorVir {
        int camadaDe = 0;
        int de = 0;
        int camada = 0;
        int ate = 0;
        float peso = 0.0f;
        bool atalho = false;
        bool leva = true;
        float sai = 0.0f;
    };
    std::vector<PorVir> mPorVir;
    [[nodiscard]] static Vector2 NoCaminho(const Sinal& s, float u);
    float mProximaEntrada = 0.0f;
    float mProximoTreino = 0.0f;
    int mAlvoDoTreino = -1;
    int mTreinos = 0;
    int mDerrubados = 0;

    // ---- A repescagem: o MaVIlab ----

    void AtualizarLaboratorio(float t, float deltaTime);
    void DesmontarLaboratorio();
    [[nodiscard]] Vector2 Normalizado(Vector2 noCampo) const;
    // A segmentacao
    struct MascaraEmCampo {
        Laboratorio::Mascara m;
        float inicio = 0.0f;     ///< quando o decoder a pintou: a celula enche ate 'dispara'
        float dispara = 0.0f;
        float some = 0.0f;
        float espacamento = 40.0f;
        bool disparou = false;
        std::vector<BossProjectile*> pixels;
    };
    std::vector<MascaraEmCampo> mMascaras;
    size_t mPassoDoDecoder = 0;
    std::vector<Vector2> mAtalhos;
    /// O atalho que se ve: de onde o aluno estava (o passo anterior) ate onde ele esta, no campo.
    std::vector<std::pair<Vector2, Vector2>> mElos;
    float mElosAte = -1.0f;
    float mProximoRuido = 0.0f;
    // A recorrente
    std::vector<Laboratorio::Memoria> mMemorias;
    float mProximaPrevisao = 0.0f;
    bool mLstm = false;
    /// O rastro de cada aluno, a memoria que se ve: as ultimas posicoes, da mais velha a mais nova.
    std::vector<std::vector<Vector2>> mRastros;
    float mProximoRastro = 0.0f;
    /// A mira de cada rajada da previsao: de onde saiu ate o fantasma, por um instante.
    struct Mira {
        Vector2 de;
        Vector2 para;
        float ate = 0.0f;
    };
    std::vector<Mira> mMiras;
    // A deteccao
    struct Colega {
        BossProjectile* p = nullptr;
        Vector2 rumo;
        float rapidez = 0.0f;
        float tiros = 0.0f;      ///< os tiros por segundo de mentira (o detector tambem os conta)
        float muda = 0.0f;
        float confianca = Laboratorio::kConfiancaBase;
    };
    std::vector<Colega> mColegas;
    std::vector<float> mConfiancaDosAlunos;
    std::vector<int> mDisparosAntes;
    std::vector<Vector2> mOndeAntes;
    int mRodada = -1;
    struct Detectado {
        Vector2 centro;
        std::string nome;
        int confianca = 0;
        int aluno = -1;   ///< a posicao do aluno detectado; -1, um colega
    };
    std::vector<Detectado> mDetectados;
    bool mCercou = false;
    /// O rastreador: ate quando ele segue cada aluno detectado (mRelogio), e o proximo tiro dele.
    std::vector<float> mRastreioAte;
    float mProximoRastreio = 0.0f;
    int mAlunosDetectados = 0;
    int mColegasDetectados = 0;

    // ---- As cores (as mesmas de gerar_michel.py) ----

    static constexpr SDL_Color kGrade = {120, 150, 190, 46};
    static constexpr SDL_Color kRaio = {255, 236, 160, 70};
    static constexpr SDL_Color kSeta = {150, 210, 255, 150};
    static constexpr SDL_Color kBrilho = {110, 220, 255, 255};
    static constexpr SDL_Color kJanela = {255, 214, 90, 230};
    static constexpr SDL_Color kAtivo = {255, 160, 60, 255};
    static constexpr SDL_Color kBorda = {255, 96, 210, 235};
    static constexpr SDL_Color kLigacao = {170, 190, 255, 255};
    static constexpr SDL_Color kAtalho = {255, 205, 90, 200};
    static constexpr SDL_Color kPerigo = {255, 80, 90, 255};
    static constexpr SDL_Color kFantasma = {200, 140, 255, 200};
    static constexpr SDL_Color kCaixa = {140, 255, 170, 200};
};
