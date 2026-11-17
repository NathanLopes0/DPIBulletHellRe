//
// O chefe da INF 216 - Projeto e Implementacao de Jogos Digitais, a optativa secreta.
//

#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

#include <SDL_pixels.h>

#include "../Boss.h"
#include "../../../Attacks/JogosDigitais.h"

class BossProjectile;
class DrawLinhasComponent;
class EtiquetaDeCodigo;
class Player;

/// O professor de jogos digitais (INF 216, a optativa secreta), o mais dificil de todos. Ele nao
/// ensina materia: pega o jogo que o aluno ja conhece e mexe nos sistemas dele, um por etapa.
///   1. a interface - a barra da nota sai do rodape e varre o campo, cheia ate a nota do aluno
///      (sobe e desce; depois, de pe, vai de um lado ao outro); e as janelas de dica, que abrem em
///      volta do aluno e so fecham com um tiro no X;
///   2. as regras - a inversao (acertar tira nota, os tiros dao), a gravidade (o aluno cai, o chao
///      tem espinhos, os tiros vem em arco) e o fogo amigo (o tiro do aluno que erra volta por baixo);
///   3. o motor - o lag (os tiros andam aos saltos), a hitbox que descola do desenho, e o quick
///      save / quick load, que traz de volta o aluno, os tiros e a nota;
///   e a repescagem, o lancamento: o jogo trava e pede uma escolha, uma frota de bugs marcha, e os
///   creditos sobem.
/// As contas estao em JogosDigitais.h; aqui, os tiros no campo.
class Ferreira : public Boss {
public:
    explicit Ferreira(Scene* scene);
    ~Ferreira() override = default;

    void OnUpdate(float deltaTime) override;
    [[nodiscard]] std::optional<Vector2> AlvoDoMovimento() const override;
    void OnDerrubou(BossProjectile* qual) override;
    void AoSerPerguntado() override;
    [[nodiscard]] float FatorDoGanho() const override { return JogosDigitais::kFatorDoGanho; }
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
    HudHostil::Degrau mDegrauDoHud;
    RegrasTrocadas::Degrau mDegrauDasRegras;
    MotorQuebrado::Degrau mDegrauDoMotor;
    Lancamento::Degrau mDegrauDoLancamento;

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
    /// Uma peca parada que o professor conduz (um segmento, uma borda, um bloco): nao sai do campo sozinha.
    BossProjectile* Peca(const char* gatilho, Vector2 onde, const std::string& cara);
    /// Os 'ps' de um disparo de 'centro' e 'abertura' (graus) ja andando, como o WaveAttack os poria.
    static void DarVelocidade(const std::vector<BossProjectile*>& ps, float centro, float abertura, float velocidade);
    /// Um anel inteiro e um leque mirado no aluno, saindo do professor.
    std::vector<BossProjectile*> Anel(const char* gatilho, int quantos, float velocidade, const std::string& cara);
    std::vector<BossProjectile*> Leque(const char* gatilho, int quantos, float abertura, float velocidade,
                                       const std::string& cara);
    [[nodiscard]] float AnguloAteOAluno(Vector2 de) const;
    [[nodiscard]] static bool Vivo(const BossProjectile* p);
    static void Matar(BossProjectile*& p);
    static void MatarTodos(std::vector<BossProjectile*>& ps);
    static void Cara(BossProjectile* p, const std::string& cara);
    static void Parar(BossProjectile* p);
    static void Andar(BossProjectile* p, Vector2 velocidade);
    [[nodiscard]] static Vector2 VelocidadeDe(const BossProjectile* p);
    [[nodiscard]] std::vector<Player*> Jogadores() const;
    [[nodiscard]] static float RaioDe(const Player* aluno);
    [[nodiscard]] bool NoCampo(Vector2 p, float folga = 0.0f) const;
    [[nodiscard]] float Nota() const;

    /// O pool recicla o projetil que morre. No comeco de cada quadro, antes de qualquer disparo, todo
    /// ponteiro guardado de um tiro morto e esquecido: o que sobra e meu e esta vivo.
    void Sanear();
    static void Sanear(std::vector<BossProjectile*>& ps, bool encolher);

    std::string mFase;
    int mEtapaAnterior = -1;
    /// Se a etapa mudou: a de antes e desmontada.
    bool TrocouDeEtapa(int etapa);
    void DesmontarEtapa();
    void Desmontar();

    DrawLinhasComponent* mLinhas = nullptr;
    static constexpr int kOrdemDasLinhas = 82;
    void Linha(Vector2 de, Vector2 ate, SDL_Color cor);
    void Tracejada(Vector2 de, Vector2 ate, SDL_Color cor, float traco = 8.0f);
    void Circulo(Vector2 centro, float raio, SDL_Color cor, int lados = 16);
    void Retangulo(Vector2 de, Vector2 ate, SDL_Color cor);
    void Preencher(Vector2 de, Vector2 ate, SDL_Color cor);
    void Seta(Vector2 de, Vector2 ate, SDL_Color cor);

    /// O texto no campo (a dica de uma janela, o dialogo da travada, uma linha dos creditos): as
    /// etiquetas nascem na primeira vez e depois so se escondem.
    EtiquetaDeCodigo* Etiqueta(std::vector<EtiquetaDeCodigo*>& grupo, size_t k, int largura, int altura);
    void Escrever(EtiquetaDeCodigo* e, const std::string& texto, Vector2 onde);
    static void Esconder(std::vector<EtiquetaDeCodigo*>& grupo);
    std::vector<EtiquetaDeCodigo*> mEtiquetasDasJanelas;
    std::vector<EtiquetaDeCodigo*> mEtiquetasDoDialogo;
    std::vector<EtiquetaDeCodigo*> mEtiquetasDosCreditos;
    static constexpr int kOrdemDasEtiquetas = 98;

    void PlacarDaEtapa(const std::string& fase, size_t etapa, const std::string& texto);
    void Avisar(const std::string& texto, float duracao = 1.6f);
    void Placar(const std::string& fase, size_t etapa, const std::string& texto);
    std::string mAviso;
    float mAvisoAte = -1.0f;
    float mRelogio = 0.0f;
    std::optional<Vector2> mAlvo;
    /// O professor varre o alto: 'periodo' segundos de um lado ao outro e de volta.
    void Varrer(float t, float amplitude, float periodo);

    /// Os tiros soltos da etapa em curso (os aneis e os leques), para quem precisa deles depois.
    std::vector<BossProjectile*> mTiros;
    float mProximoAnel = 0.0f;
    float mProximoLeque = 0.0f;
    /// O clarao de um instante (o load, a travada): a tela inteira, que se apaga.
    float mClarao = 0.0f;
    SDL_Color mCorDoClarao{255, 255, 255, 255};

    // ---- Fase 1: a interface ----

    void AtualizarHud(float t, float deltaTime);
    void AtualizarBarra(float te, bool emPe);
    std::vector<BossProjectile*> mBarra;
    struct Janela {
        Vector2 centro;
        float abre = 0.0f;
        bool aberta = false;
        std::vector<BossProjectile*> pecas;
        BossProjectile* fechar = nullptr;
        int dica = 0;
        size_t etiqueta = 0;
    };
    void AtualizarJanelas(float te);
    void FecharJanela(size_t k, bool comEstouro);
    std::vector<Janela> mJanelas;
    float mProximaJanela = 0.0f;
    int mJanelasFeitas = 0;

    // ---- Fase 2: as regras ----

    void AtualizarRegras(float t, float deltaTime);
    void AtualizarInversao(float te);
    /// Se o acerto no professor esta tirando nota (AoSerPerguntado).
    bool mCobrando = false;
    float mPremioDasMoedas = 0.0f;
    void AtualizarGravidade(float te, float deltaTime);
    std::vector<BossProjectile*> mEspinhos;
    struct Arco {
        BossProjectile* p = nullptr;
        Vector2 v;
    };
    std::vector<Arco> mArcos;
    float mProximoArremesso = 0.0f;
    void AtualizarFogoAmigo(float te);
    std::unordered_set<std::uint64_t> mVoltaram;
    struct QueDaAVolta {
        BossProjectile* p = nullptr;
        bool voltou = false;
    };
    std::vector<QueDaAVolta> mQueDaoAVolta;
    void LimparClasses();

    // ---- Fase 3: o motor ----

    void AtualizarMotor(float t, float deltaTime);
    void AtualizarLag(float te);
    struct AosSaltos {
        BossProjectile* p = nullptr;
        Vector2 onde;
        Vector2 v;
    };
    std::vector<AosSaltos> mAosSaltos;
    int mQuadroDoLag = -1;
    void AtualizarHitbox(float te);
    struct Descolado {
        BossProjectile* desenho = nullptr;
        BossProjectile* colisor = nullptr;
    };
    std::vector<Descolado> mDescolados;
    void AtualizarSaveLoad(float te);
    struct Salvo {
        bool tem = false;
        float nota = 0.0f;
        std::vector<Vector2> alunos;
        struct Tiro {
            Vector2 onde;
            Vector2 v;
        };
        std::vector<Tiro> tiros;
    };
    Salvo mSalvo;
    std::vector<BossProjectile*> mDisquetes;
    float mTeAnterior = -1.0f;

    // ---- A repescagem: o lancamento ----

    void AtualizarLancamento(float t, float deltaTime);
    void AtualizarCrash(float te);
    void Travar(int k);
    void Destravar(Lancamento::Escolha escolha);
    struct Travado {
        BossProjectile* p = nullptr;
        Vector2 v;
    };
    std::vector<Travado> mTravados;
    std::array<bool, 2> mTravou{};
    int mTravadaAberta = -1;
    /// O botao que o tiro derrubou (OnDerrubou): a escolha vale no quadro seguinte.
    Lancamento::Escolha mEscolhaPendente = Lancamento::Escolha::Nenhuma;
    std::array<BossProjectile*, 2> mBotoes{};   ///< 0: ESPERAR; 1: FECHAR
    void AtualizarFrota(float te);
    std::vector<BossProjectile*> mFrota;
    bool mFrotaMontada = false;
    bool mFrotaLimpa = false;
    int mBugsDerrubados = 0;
    float mProximaQueda = 0.0f;
    void AtualizarCreditos(float te, float deltaTime);
    struct LinhaDeCredito {
        float y = 0.0f;
        int vao = 0;
        int texto = 0;
        size_t etiqueta = 0;
        std::vector<BossProjectile*> blocos;
        std::vector<int> lugares;   ///< o indice do bloco de cada projetil
    };
    std::vector<LinhaDeCredito> mCreditos;
    int mLinhasFeitas = 0;
    float mProximaLinha = 0.0f;
    /// Quantas linhas dos creditos cabem no campo de uma vez (com folga).
    static constexpr size_t kEtiquetasDosCreditos = 8;

    // ---- As cores ----

    static constexpr SDL_Color kGuia = {235, 240, 255, 110};
    static constexpr SDL_Color kSeguro = {90, 255, 150, 22};
    static constexpr SDL_Color kVazio = {235, 240, 255, 70};
    static constexpr SDL_Color kPerigo = {255, 90, 80, 210};
    static constexpr SDL_Color kOuro = {255, 214, 90, 220};
    static constexpr SDL_Color kDebug = {120, 255, 120, 230};
    static constexpr SDL_Color kDebugFraco = {120, 255, 120, 80};
    static constexpr SDL_Color kFantasma = {200, 170, 255, 150};
    static constexpr SDL_Color kFantasmaFraco = {200, 170, 255, 60};
    static constexpr SDL_Color kJanelaPorDentro = {14, 16, 30, 150};
    static constexpr SDL_Color kSave = {110, 170, 255, 220};
};
