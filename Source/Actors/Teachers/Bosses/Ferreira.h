//
// O chefe da INF 216 - Projeto e Implementacao de Jogos Digitais, a optativa secreta.
//

#pragma once

#include <array>
#include <cstdint>
#include <memory>
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
class FaixaDaNota;
class PainelDeMenu;
class Player;

/// O professor de jogos digitais (INF 216, a optativa secreta), o mais dificil de todos. Ele nao
/// ensina materia: pega o jogo que o aluno ja conhece e mexe nos sistemas dele.
///   1. a interface - a abertura: ele tira o numero da sua nota da HUD, reescreve para 15 e o
///      devolve (a materia comeca com 15); a barra da nota, a da HUD de
///      verdade, arrancada do rodape para varrer o campo (cheia ate a nota do aluno: deitada sobe e
///      desce, de pe vai de um lado ao outro, e volta para o lugar); a camera, no manche (o aluno
///      fica fixo e o mundo anda), em tomadas: ele pausa o jogo - a pausa da batalha, com a linha
///      dele - e mexe nas opcoes (inverte os eixos; depois a suavizacao e o tremor), e o cenario
///      rola, com paredes que so se passam levando o vao ate o aluno; e o primeiro inimigo, um
///      fantasma de fliperama gigante e sem textura, com a IA a mostra - ele persegue, o power-up o
///      faz fugir (e cada bloco dele vira nota), um tiro no olho o atordoa, e no meio da etapa o
///      professor reescreve a IA com o jogo rodando (a investida, depois a previsao);
///   2. a fisica - a prova inteira e jogo de plataforma, e ele escolhe para que lado a gravidade
///      puxa: o chao de sempre (pular as pilhas de caixas), a parede esquerda (as lancas em duas
///      alturas), o teto (a chuva de blocos e moedas) e a parede direita (a esteira ate o moedor);
///   3. as regras - o fantasma da fase 1 volta nas tres primeiras: a inversao (acertar tira nota,
///      os tiros dao, e o fantasma vira moeda e foge), o fogo amigo (o tiro do aluno que erra volta
///      por baixo), o FPS (o vermelho foi feito sem deltaTime e corre com o FPS, o azul nao; o foco
///      derruba o FPS, e o fantasma tambem anda sem deltaTime) e o quick save: o disquete guarda a
///      nota e anda - foge se ela e maior que a de agora (encurralado num canto ele trava, e o tiro o
///      faz engasgar), persegue se e menor -, e quem ele encosta volta para o save, com a nota dele;
///   e a repescagem, o lancamento: a hitbox descola do desenho, o jogo trava e pede uma escolha, uma
///   frota de bugs marcha, e os creditos sobem.
/// As contas estao em JogosDigitais.h; aqui, os tiros no campo.
class Ferreira : public Boss {
public:
    explicit Ferreira(Scene* scene);
    ~Ferreira() override;

    void OnUpdate(float deltaTime) override;
    [[nodiscard]] std::optional<Vector2> AlvoDoMovimento() const override;
    void OnDerrubou(BossProjectile* qual) override;
    void AoSerPerguntado() override;
    /// Na janela da frota (o god mode caiu), cada acerto vale Lancamento::kFatorDaJanela.
    [[nodiscard]] float FatorDoGanho() const override {
        return JogosDigitais::kFatorDoGanho * (mJanelaAberta ? Lancamento::kFatorDaJanela : 1.0f);
    }
    /// O god mode da frota: enquanto houver bug, o tiro nele nao conta. Com o menu da camera aberto
    /// o jogo esta pausado, e ele tambem nao leva tiro.
    [[nodiscard]] bool AceitaDano() const override { return !mGodMode && mMenuDaCamera < 0; }
    /// Se o manche move a camera ao contrario agora: o piloto da simulacao (PilotoDoAluno) inverte o
    /// que pede, como quem ja se acostumou - a medida do 'esquiva' e o teto de quem so desvia.
    [[nodiscard]] bool CameraInvertida() const { return mCameraLigada && mMenuDaCamera < 0 && mOpcoesDaCamera.inverter; }
    void AoTerminarABatalha() override;

protected:

private:
    // ---- A luta sorteada ----

    void SortearALuta();
    bool mLutaSorteada = false;
    bool mDificil = false;
    Variacao::Sorteio mSorteio{0u};
    /// O da fisica e outro: a ordem dos lados e os desafios de cada um nao mudam quando as outras
    /// provas mudam.
    Variacao::Sorteio mSorteioDaFisica{0u};
    HudHostil::Degrau mDegrauDoHud;
    IaDoInimigo::Degrau mDegrauDoInimigo;
    Gravidade::Degrau mDegrauDaGravidade;
    RegrasTrocadas::Degrau mDegrauDasRegras;
    Lancamento::Degrau mDegrauDoLancamento;

    // ---- O que as fases dividem ----

    /// O primeiro que nasceu, ja com a cara. Com 'velocidade', nasce andando.
    BossProjectile* Um(const char* gatilho, Vector2 onde, const std::string& cara,
                       std::optional<Vector2> velocidade = std::nullopt);
    /// Uma peca parada que o professor conduz (um segmento, uma borda, um bloco): nao sai do campo sozinha.
    BossProjectile* Peca(const char* gatilho, Vector2 onde, const std::string& cara);
    /// Uma peca que anda sozinha, em linha reta: o motor a move, e quem olha o campo (o piloto da
    /// simulacao) ve para onde ela vai.
    BossProjectile* PecaAndando(const char* gatilho, Vector2 onde, const std::string& cara, Vector2 velocidade);
    /// Os 'ps' de um disparo de 'centro' e 'abertura' (graus) ja andando, como o WaveAttack os poria.
    static void DarVelocidade(const std::vector<BossProjectile*>& ps, float centro, float abertura, float velocidade);
    /// Um anel inteiro e um leque mirado no aluno, saindo do professor.
    std::vector<BossProjectile*> Anel(const char* gatilho, int quantos, float velocidade, const std::string& cara);
    std::vector<BossProjectile*> Leque(const char* gatilho, int quantos, float abertura, float velocidade,
                                       const std::string& cara);
    [[nodiscard]] float AnguloAteOAluno(Vector2 de) const;
    [[nodiscard]] static Vector2 VelocidadeDe(const BossProjectile* p);
    [[nodiscard]] std::vector<Player*> Jogadores() const;
    [[nodiscard]] static float RaioDe(const Player* aluno);
    [[nodiscard]] bool NoCampo(Vector2 p, float folga = 0.0f) const;
    [[nodiscard]] float Nota() const;
    /// O campo, para a camada pura.
    [[nodiscard]] JogosDigitais::Campo CampoPuro() const;
    /// A faixa da nota da batalha: a barra que a fase 1 arranca.
    [[nodiscard]] FaixaDaNota* Faixa() const;

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

    static constexpr int kOrdemDasLinhas = 82;
    /// O chao do jogo de plataforma vai embaixo de tudo o que a HUD do campo mostra (o placar e 60).
    DrawLinhasComponent* mLinhasDoChao = nullptr;
    static constexpr int kOrdemDoChao = 55;
    /// O contador de FPS e um overlay: vai por cima de tudo, o cartao do professor inclusive.
    DrawLinhasComponent* mLinhasDoContador = nullptr;
    static constexpr int kOrdemDoContador = 160;
    void Preencher(Vector2 de, Vector2 ate, SDL_Color cor);
    void Seta(Vector2 de, Vector2 ate, SDL_Color cor);
    /// A seta grossa da virada da gravidade, no meio do campo.
    void SetaGrande(Vector2 centro, Vector2 direcao, float tamanho, SDL_Color cor);

    /// O texto no campo (a dica de uma janela, o dialogo da travada, uma linha dos creditos): as
    /// etiquetas nascem na primeira vez e depois so se escondem.
    EtiquetaDeCodigo* Etiqueta(std::vector<EtiquetaDeCodigo*>& grupo, size_t k, int largura, int altura);
    void Escrever(EtiquetaDeCodigo* e, const std::string& texto, Vector2 onde);
    static void Esconder(std::vector<EtiquetaDeCodigo*>& grupo);
    std::vector<EtiquetaDeCodigo*> mEtiquetasDoInimigo;   ///< o estado em cima dele, e o "POWER-UP"
    std::vector<EtiquetaDeCodigo*> mEtiquetasDoSave;
    std::vector<EtiquetaDeCodigo*> mEtiquetasDoDialogo;
    std::vector<EtiquetaDeCodigo*> mEtiquetasDosCreditos;
    static constexpr int kOrdemDasEtiquetas = 98;

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
    void AtualizarAbertura(float te);
    /// A abertura reescreve a nota uma vez por luta; a de antes e a que o numero mostra saindo.
    bool mNotaEditada = false;
    float mNotaAntesDaEdicao = -1.0f;
    void AtualizarBarra(int etapa, float te);

    // ---- A camera (HudHostil::Camera) ----
    void AtualizarCamera(float te, float deltaTime);
    /// Os passos dela: o aluno desliza ate a ancora e fica preso; o manche move o mundo; o visor, a grade
    /// e o aviso; e os tiros e as caixas paradas.
    void PrenderNaCamera(float te);
    void MoverOMundo(float te, float deltaTime);
    void DesenharACamera(float te, HudHostil::Camera momento);
    void AtirarNaCamera(float te, HudHostil::Camera momento);
    /// O menu do professor (HudHostil::MenuDaCamera): o jogo pausa - o campo congela -, a pausa da
    /// batalha abre com a linha dele, e o cursor dele muda as opcoes da camera. Fecha e devolve o campo.
    void AtualizarOMenuDaCamera(float te, float deltaTime);
    void PausarOCampo();
    void RetomarOCampo();
    void FecharOMenuDaCamera();
    /// O ponteiro do mouse dele, ao lado da linha em foco, por cima do menu.
    void DesenharOPonteiro(Vector2 ponta, bool clicando);
    /// As opcoes que valem agora, a velocidade da camera (com a suavizacao, ela escorrega) e o tremor.
    HudHostil::OpcoesDaCamera mOpcoesDaCamera;
    Vector2 mVelocidadeDaCamera = Vector2::Zero;
    Vector2 mTremorAntes = Vector2::Zero;
    float mTremorDesde = -1.0f;
    float mUltimoTremor = -1.0f;
    /// O menu aberto (-1: nenhum), os cliques que ja tocaram, e o ponteiro (que desliza ate o foco).
    int mMenuDaCamera = -1;
    int mCliquesOuvidos = 0;
    Vector2 mPonteiro = Vector2::Zero;
    float mClique = 0.0f;
    std::unique_ptr<PainelDeMenu> mPausaDoProfessor;
    std::unique_ptr<PainelDeMenu> mOpcoesDoProfessor;
    /// O que o menu congelou: cada tiro com a velocidade que tinha.
    struct Congelado {
        BossProjectile* p = nullptr;
        Vector2 v;
    };
    std::vector<Congelado> mCongelados;
    /// As linhas por cima do menu (o ponteiro): a pausa e o resto da HUD ficam embaixo.
    DrawLinhasComponent* mLinhasDoMenu = nullptr;
    static constexpr int kOrdemDoMenu = 880;
    /// O quanto a camera andou desde o comeco da etapa: a grade do fundo anda com ela. 'mCamera' e o
    /// que se ve (com o tremor); 'mCameraNoMundo', sem ele - e e ele que fica nos limites do mundo.
    Vector2 mCamera = Vector2::Zero;
    Vector2 mCameraNoMundo = Vector2::Zero;
    /// De onde cada aluno desliza ate a ancora, no fim do aviso.
    std::vector<Vector2> mDeslizaDe;
    /// As caixas paradas do mundo: so a camera as move.
    struct CaixaDaCamera {
        BossProjectile* p = nullptr;
        float nasceu = 0.0f;
    };
    std::vector<CaixaDaCamera> mCaixasDaCamera;
    float mProximaCaixa = 0.0f;
    /// As paredes do cenario (HudHostil::CaixasDaParede): presas no mundo, descem com um vao.
    struct ParedeDaCamera {
        std::vector<BossProjectile*> caixas;
        float nasceu = 0.0f;
    };
    std::vector<ParedeDaCamera> mParedesDaCamera;
    float mProximaParedeDaCamera = 0.0f;
    bool mCameraAvisada = false;
    bool mCameraLigada = false;
    std::vector<EtiquetaDeCodigo*> mEtiquetasDaCamera;
    /// O visor nas quinas do campo, na cor 'cor'.
    void DesenharOVisor(SDL_Color cor);
    void TocarSom(const char* som);
    /// A barra volta para o rodape, e os segmentos embaixo dela somem.
    void DevolverABarra();
    std::vector<BossProjectile*> mBarra;
    /// A pose da barra no ultimo quadro, e quantos segmentos ela tinha: de onde saem os cacos.
    HudHostil::PoseDaBarra mPoseDaBarra;
    int mSegmentosDaBarra = 0;
    /// A brincadeira (HudHostil::Cacos): a onda do Ponto Extra dissolveu um segmento da barra. A
    /// quebra comeca no quadro seguinte, e dali em diante e so ela: 'mQuebrada' conta os segundos.
    bool mBarraAcertada = false;
    float mQuebrada = -1.0f;
    float mNotaAntesDaQuebra = 0.0f;
    std::vector<HudHostil::Caco> mCacos;
    SDL_Color mCorDosCacos{200, 150, 0, 255};
    void QuebrarABarra();
    void AtualizarQuebra(float deltaTime);
    /// O inimigo gigante (IaDoInimigo): a maquina de estados decide, e aqui o corpo anda. Na fase 1 ele
    /// tem o power-up e os leques do professor; nas regras, nao - a inversao o assusta, e no FPS ele anda sem
    /// deltaTime.
    struct ComoOInimigo {
        bool fase1 = true;      ///< o power-up e os leques do professor sao dele
        float passo = 1.0f;     ///< o quanto ele anda por segundo de deltaTime (o FPS)
        float susto = 0.0f;     ///< > 0: um susto de fora, agora, deste tamanho (a inversao)
    };
    void AtualizarInimigo(float te, float deltaTime, const ComoOInimigo& como);
    /// Os passos dele, na ordem: o power-up (se alguem o pegou), o corpo que acertou o aluno, o andar
    /// (a ronda, a perseguicao, a fuga), o corpo e os olhos no lugar (e os blocos que viram nota), a IA a
    /// mostra e os leques do professor na fase 1.
    bool PegarOPoderDoInimigo(float te, const ComoOInimigo& como);
    bool CorpoDoInimigoAcertou();
    void AndarOInimigo(Vector2 alvoDaIa, float deltaTime, const ComoOInimigo& como);
    void PorOInimigoNoLugar(Vector2 ondeAluno);
    void MostrarAIaDoInimigo(float te, const Player* alvo, Vector2 ondeAluno, Vector2 alvoDaIa);
    void LequesDoInimigo(float te, const ComoOInimigo& como);
    /// O hot reload (IaDoInimigo::kRecargas): a IA muda com o jogo rodando, e o painel de codigo
    /// escreve, letra por letra, o que mudou.
    void AoRecarregarAIa(int versao);
    void MostrarARecarga(float te);
    std::vector<EtiquetaDeCodigo*> mEtiquetasDaRecarga;
    /// A investida chegou na borda da arena (a IA percebe no quadro seguinte).
    bool mInimigoBateu = false;
    /// Nas regras, o inimigo passa de uma etapa para a outra: a troca nao o desmonta.
    bool mInimigoFica = false;
    /// O corpo inteiro e os olhos, no ninho: ele nasce.
    void MontarOInimigo();
    /// Os olhos que faltam (o tiro derrubou), de volta no lugar.
    void PorOsOlhos();
    void DesmontarOInimigo();
    /// Quando o estado muda: o que cada troca faz no campo.
    void AoTrocarDeEstado(IaDoInimigo::Estado de, IaDoInimigo::Estado para, float te);
    [[nodiscard]] int BlocosVivos() const;
    IaDoInimigo::Ia mIa;
    Vector2 mCentroDoInimigo;
    Vector2 mVelocidadeDoInimigo;
    int mCantoDaRonda = 0;
    /// Um por bloco, na ordem de IaDoInimigo::LugaresDosBlocos (nullptr: pego, ou ainda por nascer).
    std::vector<BossProjectile*> mCorpo;
    bool mCorpoMontado = false;
    std::array<BossProjectile*, 2> mOlhos{};
    /// O tiro derrubou um olho (OnDerrubou): a IA percebe no quadro seguinte.
    bool mOlhoAtingido = false;
    /// Se cada aluno ja estava piscando (acertado) no quadro anterior: o acerto e a borda.
    std::array<bool, 2> mApanhava{};
    BossProjectile* mPoder = nullptr;
    float mProximoPoder = -1.0f;
    float mPremioDoInimigo = 0.0f;
    /// O "!" ou o "?" em cima dele, por um instante, no lugar do nome do estado.
    std::string mMarca;
    float mMarcaAte = -1.0f;

    // ---- Fase 2: a fisica ----

    void AtualizarFisica(float t, float deltaTime);
    /// Sorteia o que o lado da etapa vai por no campo.
    void PrepararLado(int etapa);
    /// Todos os alunos voltam a voar.
    void SoltarAGravidade();
    /// O tamanho do desenho do aluno: o centro dele para a isto da borda (Player::PrenderNoCampo).
    [[nodiscard]] float ChaoDoAluno() const;
    void DesenharOChao(Plataforma::Lado l, float chao, float pes, SDL_Color cor, bool aviso);
    void AtualizarPulos(float te, float dt, Plataforma::Lado l, float chao, float pes);
    void AtualizarLancas(float te, float dt, Plataforma::Lado l, float chao, float pes);
    void AtualizarChuva(float te, float dt, Plataforma::Lado l, float chao, float pes);
    void AtualizarEsteira(float te, float dt, Plataforma::Lado l, float chao, float pes);
    std::array<Plataforma::Lado, 4> mOrdem{};
    /// A etapa cuja gravidade ja vale - a virada em que o professor ja pulou para o ceu dela.
    int mVirada = -1;
    struct PilhaViva {
        std::vector<BossProjectile*> caixas;
        float u = 0.0f;
        float sentido = 1.0f;
        float velocidade = 0.0f;
        int altura = 0;
        int largura = 0;
    };
    std::vector<Gravidade::Pilha> mPilhasPorVir;
    size_t mProximaPilha = 0;
    std::vector<PilhaViva> mPilhas;
    struct LancaViva {
        BossProjectile* p = nullptr;
        float u = 0.0f;
        float sentido = 1.0f;
        float velocidade = 0.0f;
        float h = 0.0f;
    };
    std::vector<Gravidade::Lanca> mLancasPorVir;
    size_t mProximaLanca = 0;
    std::vector<LancaViva> mLancas;
    struct QuedaViva {
        BossProjectile* p = nullptr;
        float u = 0.0f;
        float h = 0.0f;
        bool moeda = false;
    };
    std::vector<Gravidade::Queda> mQuedasPorVir;
    size_t mProximaDaChuva = 0;
    std::vector<QuedaViva> mQuedas;
    struct FagulhaViva {
        BossProjectile* p = nullptr;
        float u = 0.0f;
        float sentido = 1.0f;
    };
    std::vector<FagulhaViva> mFagulhas;
    struct BuracoVivo {
        std::vector<BossProjectile*> espinhos;
        float u = 0.0f;
    };
    std::vector<Gravidade::Buraco> mBuracosPorVir;
    size_t mProximoBuraco = 0;
    std::vector<BuracoVivo> mBuracos;
    std::vector<BossProjectile*> mMoedor;
    float mSentidoDaEsteira = 1.0f;

    // ---- Fase 3: as regras ----

    void AtualizarRegras(float t, float deltaTime);
    void AtualizarInversao(float te);
    /// Se o acerto no professor esta tirando nota (AoSerPerguntado).
    bool mCobrando = false;
    float mPremioDasMoedas = 0.0f;
    void AtualizarFogoAmigo(float te);
    std::unordered_set<std::uint64_t> mVoltaram;
    struct QueDaAVolta {
        BossProjectile* p = nullptr;
        bool voltou = false;
    };
    std::vector<QueDaAVolta> mQueDaoAVolta;
    void LimparClasses();
    void AtualizarFps(float te, float deltaTime);
    /// O FPS desta etapa, e o que ele foi nos ultimos segundos (o grafico embaixo do contador).
    float mFps = RegrasTrocadas::kFpsDeReferencia;
    RegrasTrocadas::Folga mFolgaDoFoco;
    /// O foco de cada aluno, para o impulso de quem o solta.
    std::array<RegrasTrocadas::Arranque, 2> mArranques{};
    std::vector<float> mHistoricoDoFps;
    float mProximaAmostra = 0.0f;
    /// O relogio de quem foi feito sem deltaTime: anda SemDeltaTime(FPS) segundos por segundo.
    float mRelogioSemDt = 0.0f;
    float mProximaEspiral = 0.0f;
    float mGiroDaEspiral = 0.0f;
    float mProximaParede = 0.0f;
    float mVaoDaParede = -1.0f;
    /// Um tiro vermelho e a velocidade dele a kFpsDeReferencia: a de agora e ela vezes o FPS.
    struct Vermelho {
        BossProjectile* p = nullptr;
        Vector2 v;
    };
    std::vector<Vermelho> mVermelhos;
    static constexpr size_t kAmostrasDoGrafico = 64;
    /// O contador: digitos e letras de sete segmentos, de 'altura' px, a partir do canto de cima a
    /// esquerda. Devolve a largura escrita.
    float SeteSegmentos(const std::string& texto, Vector2 canto, float altura, SDL_Color cor);
    void AtualizarSaveLoad(float te, float deltaTime);
    /// O quick load: quem o save encostou volta para onde estava, e a nota volta a ser a dele.
    void Carregar(float te);
    struct Salvo {
        bool tem = false;
        float nota = 0.0f;
        float ha = 0.0f;   ///< segundos desde o save (kGravando: ainda nao carrega)
        std::vector<Vector2> alunos;
    };
    Salvo mSalvo;
    /// O disquete do save, vivo no campo, e a fuga dele (o canto que trava, o tiro que engasga).
    BossProjectile* mSave = nullptr;
    RegrasTrocadas::Fuga mFuga;
    float mUltimoLoad = -1.0f;

    // ---- A repescagem: o lancamento ----

    void AtualizarLancamento(float t, float deltaTime);
    void AtualizarHitbox(float te);
    struct Descolado {
        BossProjectile* desenho = nullptr;
        BossProjectile* colisor = nullptr;
    };
    std::vector<Descolado> mDescolados;
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
    void AtualizarFrota(float te, float deltaTime);
    std::vector<BossProjectile*> mFrota;
    bool mFrotaMontada = false;
    /// O god mode (enquanto houver bug) e a janela sem ele, depois de zerada a frota.
    bool mGodMode = false;
    bool mJanelaAberta = false;
    float mJanelaAte = -1.0f;
    int mOndaDaFrota = 0;
    float mFrotaChegaEm = -1.0f;
    float mPassosDaFrota = 0.0f;
    /// O hotfix: quando o proximo bug morto volta, e o lugar que ja pisca para ele.
    float mProximoHotfix = 0.0f;
    int mHotfixLugar = -1;
    float mHotfixEm = 0.0f;
    std::vector<EtiquetaDeCodigo*> mEtiquetasDaFrota;
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
    /// A textura que faltou: o magenta do inimigo, nas linhas da IA dele.
    static constexpr SDL_Color kMagenta = {255, 60, 230, 200};
    static constexpr SDL_Color kMagentaFraco = {255, 60, 230, 70};
    static constexpr SDL_Color kSave = {110, 170, 255, 220};
    /// O FPS: o rastro do vermelho (o tamanho e a velocidade) e o frio da camera lenta.
    static constexpr SDL_Color kRastro = {255, 110, 90, 150};
    static constexpr SDL_Color kFrio = {110, 220, 255, 235};
    /// O chao do jogo de plataforma: a faixa, a linha de cima e as juntas dos ladrilhos.
    static constexpr SDL_Color kChaoFundo = {26, 30, 54, 215};
    static constexpr SDL_Color kChaoLinha = {175, 185, 235, 235};
    static constexpr SDL_Color kChaoJunta = {95, 105, 160, 170};
};
