//
// A chefe da INF 491 - Topicos Especiais: Bioinformatica, a terceira optativa.
//

#pragma once

#include <array>
#include <optional>
#include <string>
#include <vector>

#include <SDL_pixels.h>

#include "../Boss.h"
#include "../../../Attacks/Dobramento.h"
#include "../../../Attacks/Helice.h"
#include "../../../Attacks/Pareamento.h"
#include "../../../Attacks/Sequenciamento.h"

class BossProjectile;
class DrawLinhasComponent;
class Player;

/// A professora de bioinformatica (INF 491, optativa), que le sequencias e gira moleculas em 3D.
/// Cada prova e um pedaco do caminho do DNA a proteina, e pede que quem joga leia a molecula:
///   1. a dupla helice - o modelo 3D gira, e so a frente machuca; a rosca da torcao; o calor que
///      solta A-T antes de G-C;
///   2. o codigo - o aluno carrega uma base, e a que pareia com ela nao machuca: as ondas de uma base
///      so, a transcricao (U no lugar de T) e a traducao, em que a familia do aminoacido decide o voo;
///   3. a proteina - a cadeia que persegue e se corta a tiro, o globulo que rola, e o sitio ativo
///      onde o aluno se encaixa;
///   e a repescagem, o sequenciamento: a PCR que dobra, o shotgun que monta o contig, e o CRISPR que
///   so corta onde o guia casa.
class Sabrina : public Boss {
public:
    explicit Sabrina(Scene* scene);
    ~Sabrina() override = default;

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
    Helice::Degrau mDegrauDaHelice;
    Pareamento::Degrau mDegrauDoCodigo;
    Dobramento::Degrau mDegrauDaProteina;
    Sequenciamento::Degrau mDegrauDoSequenciamento;

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
    /// Um enfeite parado: nao machuca e o tiro passa.
    BossProjectile* Enfeite(const char* gatilho, Vector2 onde, const std::string& cara);
    /// A base 'b' disparada de 'de' em leque: a cara e a letra, e a classe e a do par dela (o aluno
    /// que carrega a base que pareia passa - e absorve).
    std::vector<BossProjectile*> Bases(const char* gatilho, Vector2 de, Pareamento::Base b, int quantos,
                                       float centro, float abertura, float velocidade, bool comClasse);

    [[nodiscard]] float AnguloAteOAluno(Vector2 de) const;
    [[nodiscard]] static float AnguloAte(Vector2 de, Vector2 para);
    [[nodiscard]] static bool Vivo(const BossProjectile* p);
    static void Matar(BossProjectile*& p);
    static void Cara(BossProjectile* p, const std::string& cara);
    static void Parar(BossProjectile* p);
    static void Mover(BossProjectile* p, Vector2 onde, Vector2 velocidade);
    [[nodiscard]] std::vector<Player*> Jogadores() const;
    [[nodiscard]] bool NoCampo(Vector2 p, float folga = 0.0f) const;

    std::string mFase;
    void Desmontar();

    DrawLinhasComponent* mLinhas = nullptr;
    static constexpr int kOrdemDasLinhas = 82;
    void Linha(Vector2 de, Vector2 ate, SDL_Color cor);
    void Tracejada(Vector2 de, Vector2 ate, SDL_Color cor, float traco = 8.0f);
    void Circulo(Vector2 centro, float raio, SDL_Color cor, int lados = 16);
    void Retangulo(Vector2 de, Vector2 ate, SDL_Color cor);
    void Preencher(Vector2 de, Vector2 ate, SDL_Color cor);
    /// O termometro na beirada direita do campo: de 'minima' a 'maxima' graus, cheio ate 'agora'.
    void Termometro(float agora, float minima, float maxima);

    void PlacarDaEtapa(const std::string& fase, size_t etapa, const std::string& texto);
    void Avisar(const std::string& texto, float duracao = 1.6f);
    void Placar(const std::string& fase, size_t etapa, const std::string& texto);
    std::string mAviso;
    float mAvisoAte = -1.0f;
    float mRelogio = 0.0f;
    int mEtapaAnterior = -1;

    std::optional<Vector2> mAlvo;
    /// A professora varre o alto: 'periodo' segundos de um lado ao outro e de volta.
    void Varrer(float t, float amplitude, float periodo);

    /// A cor de cada base (A verde, C azul, G ambar, T e U vermelho), como nos cromatogramas.
    [[nodiscard]] static SDL_Color CorDa(Pareamento::Base b, Uint8 alfa = 255);

    // ---- O modelo 3D (fases 1 e 3) ----

    /// Uma conta de um modelo 3D: na frente, e um projetil que machuca, do tamanho da perspectiva;
    /// atras, so o contorno apagado (o projetil morre).
    struct Conta {
        BossProjectile* p = nullptr;
        Vector2 onde;
        bool frente = false;
    };
    /// Poe a conta em 'tela' com a escala; nasce ou morre ao trocar de lado.
    void Posicionar(Conta& c, const Helice::NaTela& tela, bool frente, const char* gatilho, const std::string& cara,
                    float deltaTime, SDL_Color contorno);
    static constexpr float kDistanciaDaCamera = 700.0f;

    // ---- Fase 1: a dupla helice ----

    void AtualizarHelice(float t, float deltaTime);
    void DesmontarHelice();
    std::vector<Pareamento::Base> mMolde;     ///< a fita 0 (a fita 1 e a complementar)
    std::vector<Conta> mContas[2];             ///< as contas de cada fita
    int mFormaDaHelice = -1;                   ///< 0: de pe; 1: deitada
    float mGiro = 0.0f;
    float mGiroAntes = 0.0f;
    std::vector<float> mSoltouEm;              ///< o mRelogio em que cada par soltou (negativo: preso)
    int mSoltos = 0;
    float mProximoLequeDaHelice = 0.0f;

    // ---- Fase 2: o codigo (e a base que o aluno carrega) ----

    void AtualizarCodigo(float t, float deltaTime);
    void DesmontarCodigo();
    /// A base de cada aluno: o selo que o segue, o anel na cor dela, as pontes de hidrogenio ate os
    /// tiros que pareiam, a absorcao, e as capsulas que trocam a base.
    void AtualizarBases(float deltaTime, bool rna);
    struct Selo {
        BossProjectile* p = nullptr;
        int classe = 0;
        float trocouEm = -10.0f;
    };
    std::vector<Selo> mSelos;
    struct Capsula {
        BossProjectile* p = nullptr;
        int classe = 0;
        float voltaEm = -1.0f;
    };
    std::vector<Capsula> mCapsulas;
    float mOrbita = 0.0f;
    int mAbsorvidas = 0;
    float mPremioGuardado = 0.0f;
    // O pareamento
    Pareamento::Base mDominante = Pareamento::Base::A;
    float mProximaOnda = 0.0f;
    int mOndas = 0;
    // A transcricao
    std::vector<Pareamento::Base> mFitaMolde;
    std::vector<BossProjectile*> mLetrasDoMolde;
    std::vector<BossProjectile*> mLetrasDoRna;
    int mLidas = 0;
    BossProjectile* mMaquina = nullptr;        ///< a polimerase, e depois o ribossomo
    // A traducao
    std::vector<Pareamento::Base> mMensageiro;
    std::vector<BossProjectile*> mLetrasDoMensageiro;
    int mCodons = 0;
    char mUltimoAminoacido = 'M';
    struct Teleguiado {
        BossProjectile* p = nullptr;
        float ate = 0.0f;
    };
    std::vector<Teleguiado> mTeleguiados;
    std::vector<BossProjectile*> mPolipeptideo;
    std::vector<Vector2> mOndeDoPolipeptideo;

    // ---- Fase 3: a proteina ----

    void AtualizarProteina(float t, float deltaTime);
    void DesmontarProteina();
    struct Residuo {
        BossProjectile* p = nullptr;
        Pareamento::Familia familia = Pareamento::Familia::Polar;
    };
    // A cadeia
    std::vector<Residuo> mCadeia;
    std::vector<Vector2> mOndeDaCadeia;
    Vector2 mRumo{0.0f, 1.0f};
    float mProximoResiduo = 0.0f;
    float mProximaCadeia = -1.0f;
    float mProximoLequeDaCadeia = 0.0f;
    int mCortes = 0;
    struct Solto {
        BossProjectile* p = nullptr;
        float morre = 0.0f;
    };
    std::vector<Solto> mSoltosDaCadeia;
    /// O residuo 'i' foi derrubado: a cadeia acaba nele, e o resto se solta.
    void CortarACadeia(size_t i);
    // O globulo
    bool mGlobuloMontado = false;
    std::vector<Helice::Ponto3> mDobrado;
    std::vector<Pareamento::Familia> mFamilias;
    std::vector<Vector2> mDeOnde;              ///< de onde cada residuo saiu para o colapso
    std::vector<Conta> mAtomos;
    Vector2 mCentroDoGlobulo;
    Vector2 mVelocidadeDoGlobulo;
    float mGiroY = 0.0f;
    float mGiroX = 0.0f;
    float mComecoDoGlobulo = 0.0f;
    float mProximoBorrifo = 0.0f;
    // O encaixe
    std::vector<BossProjectile*> mParede;
    struct Gota {
        BossProjectile* p = nullptr;
        Vector2 v;
    };
    std::vector<Gota> mGotas;
    float mProximaGota = 0.0f;
    float mProximoInibidor = 0.0f;
    float mEncaixado = 0.0f;
    float mComecoDoEncaixe = -1.0f;

    // ---- A repescagem: o sequenciamento ----

    void AtualizarSequenciamento(float t, float deltaTime);
    void DesmontarSequenciamento();
    // A PCR
    struct Copia {
        BossProjectile* p = nullptr;
        Vector2 v;
        bool dupla = true;
        BossProjectile* primer = nullptr;
        bool primerCaiu = false;
    };
    std::vector<Copia> mCopias;
    int mCicloAntes = -1;
    Sequenciamento::Passo mPassoAntes = Sequenciamento::Passo::Estende;
    int mPrimersDerrubados = 0;
    // O shotgun
    std::vector<BossProjectile*> mContig;      ///< da ponta de tras a da frente
    int mAtras = 0;
    int mFrente = 0;
    float mGiroDoContig = 0.0f;
    float mProximaRajada = 0.0f;
    std::vector<BossProjectile*> mReads;
    void MontarOContig();
    // O CRISPR
    std::vector<Pareamento::Base> mGuia;
    struct FitaDoGenoma {
        Sequenciamento::Genoma genoma;
        std::vector<BossProjectile*> contas;
        std::vector<BossProjectile*> elos;   ///< o esqueleto entre as letras: fecha a parede
        float y = 0.0f;
        bool cortada = false;
        bool foraDoAlvo = false;
        bool avisou = false;     ///< o aviso do tiro fora do alvo, uma vez por fita
    };
    std::vector<FitaDoGenoma> mGenomas;
    float mProximaFita = 0.0f;
    int mEdicoes = 0;
    BossProjectile* mTesoura = nullptr;
    float mTesouraAte = -1.0f;

    // ---- As cores (as mesmas de gerar_sabrina.py) ----

    static constexpr SDL_Color kFundo = {150, 170, 210, 70};
    static constexpr SDL_Color kEsqueleto = {170, 200, 255, 120};
    static constexpr SDL_Color kPonte = {230, 236, 255, 150};
    static constexpr SDL_Color kCalor = {255, 120, 70, 230};
    static constexpr SDL_Color kFrio = {110, 180, 255, 230};
    static constexpr SDL_Color kLigacao = {200, 205, 220, 110};
    static constexpr SDL_Color kSitio = {200, 150, 255, 210};
    static constexpr SDL_Color kOuro = {255, 214, 90, 230};
    static constexpr SDL_Color kPam = {255, 190, 60, 220};
};
