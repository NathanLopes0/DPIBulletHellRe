//
// Created by nslop on 16/09/2024.
//

#ifndef DPIBULLETHELLRE_BATTLE_H
#define DPIBULLETHELLRE_BATTLE_H

#include <memory>
#include <string>
#include <vector>
#include "../Scene.h"
#include "../../Intercambio.h"
#include "../../Game.h"
#include "../../AudioSystem.h"
#include "../../Pausa.h"
#include "../../Carregamento.h"
#include "../../Dificuldade.h"
#include "../../Hud.h"
#include "../../Variacao.h"
#include "../../Actors/Teachers/BossFactory/IBossFactory.h"

#define GRADE_CHANGE_DOWN 6.f


class Font;
class Player;
class ProjectileManager;
class BattleHUD;
class ExtraPointItem;
class Orientador;

class Battle : public Scene {

public:

    explicit Battle(Game* game, int selectedStage);
    ~Battle() override;
    void Load() override;
    void CheckCollisions();
    /**
     * @brief Uma fase do chefe acabou e outra comecou.
     *
     * Chamado pelo callback que o Boss registra na maquina de estados. Faz duas
     * coisas: reinicia a barra de tempo e FECHA a fase que terminou, pagando o
     * bonus de fase limpa quando for o caso.
     *
     * Chamava-se ResetHUDTimer e so fazia a primeira. O nome mudou junto com o
     * trabalho: um nome que descreve metade do que a funcao faz e pior que um
     * nome generico.
     */
    void OnFaseTrocada(float novaDuracao);

    [[nodiscard]] Boss* GetBoss() const { return mBoss; }

    /// O orientador do TCC, quando ha um em campo. Ver LoadOrientador.
    [[nodiscard]] Orientador* GetOrientador() const { return mOrientador; }

    /**
     * @brief Paga uma CONQUISTA DA DUPLA: um ganho bruto, inteiro (nao passa por
     * Nota::PorJogador - ver GradeUp). E o que o chefe usa para pagar o que o
     * aluno faz alem de acertar o corpo dele: o objetivo cumprido do TCC.
     */
    void Conquistar(const float bruto) { if (EmCombate()) GradeUp(bruto); }

    /// Se a luta esta valendo: nem carregando os projeteis, nem na tela de fim.
    /// Quem age por conta propria (o orientador) so age enquanto isto vale.
    [[nodiscard]] bool EmCombate() const { return !mIsLoading && !mIsEnding; }

    /**
     * @brief TODOS os corpos do chefe em campo, e nao so o original.
     *
     * Normalmente e um so. Na fase que se divide sao dois, e QUALQUER UM DOS
     * DOIS TOMA DANO - e por isso que as regras de colisao percorrem esta
     * lista em vez de perguntar a mBoss. Mesma forma que mJogadores: a batalha
     * nao pergunta "e co-op?" nem "esta dividido?", ela percorre o que existe.
     *
     * mBoss continua sendo o ORIGINAL, e e dele que saem a barra de tempo, o
     * retrato do HUD e o prewarm dos projeteis - coisas que existem uma vez por
     * luta e nao uma por corpo.
     */
    [[nodiscard]] const std::vector<Boss*>& Corpos() const { return mCorpos; }

    /**
     * @brief O modo desta batalha (Dificuldade.h).
     *
     * Lido do Game no comeco do Load, antes de o chefe nascer, e o mesmo do
     * comeco ao fim: a tela de Opcoes nao abre no meio de uma luta. E daqui que
     * cada chefe le o proprio degrau (Boss::NoNivel).
     */
    [[nodiscard]] Dificuldade::Nivel NivelDeDificuldade() const { return mNivel; }

    /**
     * @brief A semente desta luta (Variacao.h): de onde cada chefe tira o que
     * muda de uma batalha para a outra dentro de cada fase.
     *
     * Sorteada no comeco do Load e escrita no log, para que uma luta estranha
     * possa ser repetida (no arnes, DPI_VARIACAO). Recomecar a fase pela pausa
     * recarrega a cena, e a luta nova tem outra semente.
     */
    [[nodiscard]] std::uint32_t SementeDaBatalha() const { return mSemente; }

    /**
     * @brief Poe um segundo corpo em campo, com os ataques da fase nomeada.
     *
     * Chamada pelo chefe ao entrar numa fase que declara "clone" em fases.json.
     * Nao faz nada se ja houver um segundo corpo.
     */
    void DividirChefe(const std::string& faseDoClone);

    /// @brief Tira de campo os corpos extras, deixando so o original.
    void JuntarCorpos();

    /**
     * @brief OS JOGADORES EM CAMPO, na ordem das posicoes do painel.
     *
     * Um por sessao: quantos jogam foi decidido no menu, e a batalha nao
     * pergunta se e co-op - ela percorre o que existe.
     *
     * Quem quer MIRAR nao usa esta lista: usa Alvo().
     */
    [[nodiscard]] const std::vector<Player*>& Jogadores() const { return mJogadores; }

    /**
     * @brief EM QUEM UM ATAQUE MIRADO MIRA.
     *
     * Chamava-se GetPlayer(), e DOZE lugares a chamavam para esta mesma coisa:
     * onde esta o alvo. O nome mentia por omissao - "o jogador" deixa de
     * existir quando ha dois, e cada ataque teria de escolher por conta
     * propria, doze vezes, cada um do seu jeito.
     *
     * ESTAVEL DENTRO DE UM DISPARO. O laser pergunta duas vezes (o nulo e a
     * posicao) e as duas tem de dar o mesmo jogador; um laser que trocasse de
     * alvo no meio do tracado seria ruido, nao dificuldade.
     *
     * ELA NAO PASSA A VEZ. Quem passa e RevezarAlvo, uma vez por disparo: se
     * Alvo() revezasse sozinha, o laser trocaria de alvo entre a verificacao de
     * nulo e a leitura da posicao, duas linhas depois.
     *
     * Devolve nullptr quando nao ha ninguem em campo, que acontece de verdade
     * entre o Load e a criacao do jogador - os doze chamadores ja tratavam isso.
     */
    [[nodiscard]] Player* Alvo() const;

    /**
     * @brief Em qual POSICAO esta a vez.
     *
     * Quem precisa guardar o alvo guarda este numero, e NAO o ponteiro: os
     * projeteis vem de um pool que vive no Game, entao um deles pode
     * sobreviver a batalha que o criou - e um ponteiro para um jogador da
     * batalha passada seria lixo apontando para memoria liberada.
     */
    [[nodiscard]] size_t IndiceDoAlvo() const { return mVezDoAlvo; }

    /// O jogador de uma posicao, ou nullptr fora da lista. E como um projetil
    /// resolve o numero que guardou.
    [[nodiscard]] Player* JogadorEm(size_t posicao) const;

    /**
     * @brief PASSA A VEZ para o proximo jogador.
     *
     * Chamado UMA vez por disparo, em Boss::ExecuteAttack - o unico lugar por
     * onde todo ataque de todo chefe passa. Por disparo, e nao por quadro
     * (requisitos-coop, D3).
     *
     * Com um jogador em campo nao faz nada observavel.
     */
    void RevezarAlvo();

    /**
     * @brief DA A VEZ a um jogador, antes de um disparo que e DELE.
     *
     * So o padrao Observer da Maria Lucia usa: quem mudou de estado e quem
     * recebe os avisos, e nao o jogador da vez. O disparo passa a vez depois,
     * como todos (RevezarAlvo). Posicao fora da lista nao faz nada.
     */
    void DarAVez(size_t posicao);
    [[nodiscard]] ProjectileManager* GetProjectileManager() const { return mProjectileManager.get(); }

    /// @brief A fonte do placar (Carlito), para quem escreve codigo no campo
    /// (EtiquetaDeCodigo). E da batalha, que vive mais que os atores dela.
    [[nodiscard]] Font* FonteDoPlacar() const { return mFonteDoPlacar.get(); }

    SDL_FRect GetPlayfieldBounds() const override;
    /**
     * @brief Um aluno gastou um Ponto Extra.
     *
     * RECEBE QUEM GASTOU porque em dupla o efeito e no OUTRO: a tela nao e
     * limpa, o parceiro fica imune. Ver o corpo.
     */
    void OnPlayerUsedExtraPoint(Player* quemGastou);

    /// @brief O outro aluno em campo, ou nullptr se este estiver sozinho.
    [[nodiscard]] Player* OutroJogador(const Player* quem) const;
    void SpawnExtraPoint(Vector2 position);

    void FinishBattle(bool approved);

    /**
     * @brief A nota da batalha EM CURSO.
     *
     * Quem precisa dela e a maquina de estados do chefe, para decidir entre a
     * repescagem e o fim. Antes ela era lida de Game::GetGrade, que e o
     * armazenamento persistente - o que tinha dois problemas: a nota so chegava
     * la quando algo marcava 'changedGrade' (entao podia estar atrasada), e
     * escrever nela durante a batalha fazia as regras de desbloqueio oscilarem.
     */
    [[nodiscard]] float GetNotaAtual() const { return mGrade; }

    /// @brief Se esta batalha ja perdeu o direito a nota cheia por dano sofrido.
    [[nodiscard]] bool PerdeuACheia() const;


protected:
    void TimeBarUpdate() const;

    void GradeTextUpdate();
    void GradeBarUpdate();

    void OnUpdate(float deltaTime) override;
    void OnProcessInput(const Uint8 *keyState) override;

private:
    // --- Funções Auxiliares de Lógica Interna ---
    void LoadHUD();
    void LoadPlayer();
    void LoadBoss();

    /**
     * O ORIENTADOR, SE A MATERIA PEDE UM E ALGUEM FOI ESCOLHIDO. Quem escolhe e
     * a EscolhaDoOrientador, e o escolhido chega pelo Game (Game::Orientador).
     * Depois dos jogadores, porque nasce ao lado deles.
     */
    void LoadOrientador();

    void LoadTimeClock();

    void LoadTimeBar();
    void LoadGradeBar();

    // ---- O PLACAR: o que o chefe anuncia na fase ("anuncio" em fases.json) ----
    void LoadPlacar();
    void AtualizarPlacar();

    // ---- O INTERCAMBIO: os alunos trocam de lugar ("intercambio" em fases.json) ----
    void LoadIntercambio();
    void AtualizarIntercambio(float deltaTime);


    // --- Membros de Estado da Batalha ---
    float mGrade;
    int mStage{};

    /// Acertos sofridos NA FASE atual. Separado do contador da batalha porque
    /// responde outra pergunta: o da batalha decide o teto de 99,99, este decide
    /// o bonus de fase limpa. Zera a cada troca de fase.
    int mAcertosNaFase{};

    /// Fecha a fase em curso: paga o bonus se ela foi limpa e zera o contador.
    /// Chamada na troca de fase E no fim da batalha - a ultima fase nao troca
    /// para lugar nenhum, e sem isso ela nunca pagaria.
    void EncerrarFase();

    /// O callback da maquina de estados dispara tambem ao ENTRAR na primeira
    /// fase, quando nao ha fase anterior. Sem isto, a batalha comecaria pagando
    /// um bonus por uma fase que ninguem jogou.
    bool mPrimeiraFase = true;

    /// Quanto tempo o aviso de fase limpa ainda fica na tela.
    float mAvisoDeFaseLimpa{};

    Actor* mFaseLimpaActor = nullptr;

    /// Ver NivelDeDificuldade.
    Dificuldade::Nivel mNivel = Dificuldade::Nivel::Normal;

    /// Ver SementeDaBatalha.
    std::uint32_t mSemente = 0;

    /// O selo DIFICIL, na ponta direita da faixa da nota. So existe no Dificil:
    /// quem ve uma luta no corredor sabe que ela nao e a de sempre, e quem joga
    /// sabe que a opcao ficou ligada por quem jogou antes.
    Actor* mSeloDoNivel = nullptr;

    /// Quantos projeteis do chefe acertaram o jogador NESTA batalha. Zera a cada
    /// batalha junto com a nota: o teto de Nota e por batalha, nao por perfil.
    /// Passado dos 2, a nota desta batalha para em 99,99 (ver Nota.h).
    int mAcertosSofridos{};

    // ---- A FAIXA DA NOTA, na base da tela ----------------------------------
    //
    // Os numeros estavam soltos dentro de LoadGradeBar, e a altura estava
    // escrita DE NOVO em GetPlayfieldBounds, com um comentario pedindo para
    // manter as duas iguais. Pedido em comentario nao segura nada - e a conta
    // dos textos ja dependia da altura por um caminho que ninguem via.

    /// A altura da barra. Era 25, e subiu junto com a moldura: um retangulo de
    /// 25 px com 2 px de borda de cada lado sobra 21 de preenchimento.
    static constexpr int kBarraAltura = 28;

    /// A sobra dos lados. A barra ia de ponta a ponta da tela e encostava nas
    /// bordas, o que a fazia parecer um risco no monitor em vez de uma peca.
    static constexpr int kBarraMargemLateral = 40;

    /// Do fundo da tela ate o TOPO da barra.
    static constexpr int kBarraDoFundo = 54;

    /// A placa atras de um texto da faixa.
    ///
    /// EXISTE POR LEITURA, nao por enfeite: o numero e branco e o
    /// preenchimento passa por laranja - "52.64" sumia em cima dele. O mesmo
    /// vale para o FASE LIMPA, que e verde claro sobre uma barra verde.
    ///
    /// ELA TINGE, NAO TAPA. A primeira versao era opaca e da altura da barra
    /// mais dois: em vez de uma placa, lia-se um BURACO recortado no meio do
    /// preenchimento, cortando a barra em duas. Translucida e da altura exata
    /// da barra, ela escurece a faixa onde o numero esta e a barra continua
    /// sendo uma coisa so.
    /// O ganho bruto de gastar um Ponto Extra. Era um 10.f solto dentro de
    /// OnPlayerUsedExtraPoint, ao lado de Nota::kGanhoPorAcerto e GRADE_CHANGE_DOWN

    /// que ja moravam em constantes - e agora ele e repartido como os outros,
    /// entao vale ter nome.
    static constexpr float kGanhoDoPontoExtra = 10.0f;

    // --- O PONTO EXTRA DO CO-OP, A VISTA (PontoExtra.h): o feixe de quem gastou
    // ate o parceiro, e o anel do escudo em volta de quem esta protegido.
    struct FeixeDoPontoExtra {
        const Player* de = nullptr;
        const Player* ate = nullptr;
        float t = 0.0f;
    };
    std::vector<FeixeDoPontoExtra> mFeixes;
    class DrawLinhasComponent* mDesenhoSobreOsAlunos = nullptr;
    float mPulsoDoEscudo = 0.0f;

    /// Na frente do aluno (100): o escudo e o que se le, e nao o sprite.
    static constexpr int kOrdemSobreOsAlunos = 102;

    /// Anda os feixes e redesenha o que fica por cima dos alunos: os feixes,
    /// os escudos e, se a opcao estiver ligada, a hitbox de cada um.
    void AtualizarDesenhoSobreOsAlunos(float deltaTime);

    // ---- A MOLDURA DO CAMPO E OS EFEITOS (Hud.h) -------------------------------
    //
    // A moldura mostra onde o aluno para - os trilhos dos lados, o painel da
    // faixa da nota, a linha de cima - e responde: a parede acende quando ele
    // encosta, e o campo pisca na cor de cada acontecimento. As faiscas sao o
    // retorno de cada tiro: o acerto no professor, o projetil derrubado, o
    // acerto sofrido, o Ponto Extra apanhado.
    class DrawMolduraDoCampoComponent* mMoldura = nullptr;
    Hud::Caixa mAreaDoAluno;
    void LoadMoldura();
    void AtualizarMoldura();

    struct FaiscaEmCampo {
        Vector2 onde;
        Hud::Faisca tipo = Hud::Faisca::Acerto;
        float idade = 0.0f;
    };
    std::vector<FaiscaEmCampo> mFaiscas;
    class DrawLinhasComponent* mEfeitos = nullptr;
    /// Um efeito em 'onde'. Com muitos ao mesmo tempo, os mais velhos saem.
    void Faiscar(const Vector2& onde, Hud::Faisca tipo);
    void AtualizarEfeitos(float deltaTime);
    static constexpr int kOrdemDosEfeitos = 104;
    static constexpr size_t kMaximoDeFaiscas = 64;
    static constexpr int kOrdemDaMoldura = 150;



    static constexpr int kPlacaLargura = 112;

    /// A dos avisos e mais larga: "TESTE FINAL" tem onze letras contra as seis
    /// de "100.00".
    static constexpr int kPlacaDoAvisoLargura = 196;

    /// A MESMA altura da barra, e nao um numero proprio: dois pixels de
    /// diferenca ja faziam a placa aparecer por cima da moldura.
    static constexpr int kPlacaAltura  = kBarraAltura;
    static constexpr int kOrdemDaPlaca = 303;   ///< o texto da faixa usa 304
    static inline const SDL_Color kSemMoldura{0, 0, 0, 0};
    static inline const SDL_Color kFundoDaPlaca{6, 8, 16, 135};
    /// A PLACA DO NUMERO DA NOTA, mais fechada que a dos avisos e com um fio de
    /// moldura: ela fica em cima do preenchimento com volume, e a 135 o reflexo
    /// que passa na barra atravessava o numero.
    static inline const SDL_Color kFundoDaPlacaDaNota{6, 8, 16, 200};
    static inline const SDL_Color kMolduraDaPlacaDaNota{255, 255, 255, 55};

    // --- Ponteiros observadores do Boss e do Player, que vão estar dentro
    // do vetor mActors da própria cena Battle. Ideal para, mesmo num vetor de Actors,
    // chamar funções específicas de Player e de Boss, e também para acesso rápido sem percorrer o vetor.
    Boss* mBoss{};

    /// O original mais os clones vivos. Ver Corpos().
    std::vector<Boss*> mCorpos;

    /// OS JOGADORES, em vez de um. Ver Jogadores() e Alvo().
    std::vector<Player*> mJogadores;

    /// O orientador do TCC, ou nulo. O ator e da cena, como todos.
    Orientador* mOrientador = nullptr;

    /// De quem e a vez de ser mirado. Ver RevezarAlvo.
    size_t mVezDoAlvo = 0;

    // O GERENTE DE PROJÉTEIS: Um sistema especializado, possuído pela cena
    std::unique_ptr<ProjectileManager> mProjectileManager;

    // O HUD
    std::unique_ptr<BattleHUD> mHUD;
    std::unique_ptr<Font> mGradeBarFont;

    /// A FONTE DO PLACAR NAO E A ZELDA, e nao por gosto: a Zelda nao tem "*" nem
    /// "^" (nem "+" e "_", que ja viraram quadrado na tela), e "(ab)*" e "aⁿbⁿ" sao
    /// justamente o que o placar do Vladimir escreve. A Carlito tem a notacao
    /// inteira, inclusive os sobrescritos e o delta.
    std::unique_ptr<Font> mFonteDoPlacar;
    Actor* mPlacarActor = nullptr;

    /// O texto que o placar mostra agora. So e refeito quando muda: SetText
    /// renderiza uma textura nova, e fazer isso todo quadro seria pagar por um
    /// texto que nao mudou. E o TEXTO, e nao a fase, que se compara: dentro de
    /// uma fase com Intercambio o placar troca de texto durante o aviso.
    std::string mTextoDoPlacar;

    /// O relogio do Intercambio da fase atual. Sem Intercambio ele fica
    /// invalido, e um relogio invalido nunca avisa nem troca (Intercambio.h).
    Intercambio::Relogio mRelogioDoIntercambio;

    /// A fase de que o relogio e. Trocar de fase zera o relogio: o periodo
    /// conta do comeco de cada fase, e nao da batalha.
    std::string mFaseDoIntercambio;

    /// Um marcador por aluno, no lugar onde ele vai cair. So aparecem durante
    /// o aviso, e andam junto com o destino - que muda quando os alunos andam.
    std::vector<Actor*> mMarcadoresDoIntercambio;

    // O que o placar diz enquanto o aviso esta aceso e do chefe: ver
    // Boss::AvisoDoIntercambio.

    /// Logo acima dos projeteis (88 a 92) e abaixo do aluno (100): o marcador e
    /// so uma moldura vazia, entao por cima dos tiros ele nao esconde nada, e
    /// por baixo deles sumiria justo onde o campo esta cheio.
    static constexpr int kOrdemDoMarcador = 95;

    static constexpr int kPlacarLargura = 320;
    static constexpr int kPlacarAltura  = 46;
    static constexpr int kPlacarMargem  = 16;
    static constexpr int kTamanhoDoPlacar = 30;



    /// ATRAS DOS PROJETEIS (88 a 92) e do aluno (100), e de proposito. O aluno
    /// alcanca o campo inteiro, cantos de cima inclusive, e um placar por cima
    /// esconderia um tiro justo de quem esta embaixo dele. Atras, ele e
    /// CENARIO: o quadro da sala, e nao uma janela por cima do jogo.
    static constexpr int kOrdemDoPlacar = 60;
    Actor* mHUDTimeBarActor = nullptr;
    Actor* mHUDClockActor = nullptr;

    Actor* mGradeBarActor = nullptr;
    Actor* mGradeTextActor = nullptr;

    /// O "TESTE FINAL" na faixa da nota. Escondido ate a nota chegar a 100, e a
    /// partir dali fica - ver GradeTextUpdate.
    Actor* mTesteFinalActor = nullptr;

    /**
     * @brief Soma um ganho BRUTO na nota da batalha.
     *
     * SEM VALOR PADRAO, de proposito. Ha dois tipos de ganho e eles se
     * repartem de formas diferentes (requisitos-coop, 2.1): o que um jogador
     * causa passa por Nota::PorJogador, o que a dupla conquista e pago
     * inteiro. Com um padrao, a chamada mais comum - o acerto no chefe - nao
     * dizia qual dos dois era, e quem acrescentasse um ganho novo herdaria a
     * escolha sem notar que havia uma.
     */
    void GradeUp(float amount);

    /// Quanto vale, nesta batalha, algo que UM jogador causou. Ver
    /// Nota::PorJogador: com um jogador em campo, devolve o proprio bruto.
    [[nodiscard]] float PorJogador(float bruto) const;

    /// @brief O que um acerto sofrido custa: dividido E aliviado. Ver Nota.h.
    [[nodiscard]] float PerdaPorJogador(float bruto) const;
    /**
     * @brief Cobra um acerto sofrido.
     *
     * @param multiplicadorDaPerda quanto da perda normal este acerto custa. O
     *        padrao e 1 - a perda de sempre. As CORRENTES do Vitor usam um
     *        valor baixo: elas quase nao cobram nota e em troca prendem o
     *        aluno (ver o laco de colisao).
     *
     * A CONTAGEM DE ACERTOS NAO E MULTIPLICADA. Uma corrente conta como acerto
     * para tudo que conta acertos - o direito a nota cheia, a fase limpa -,
     * porque o aluno foi acertado de verdade.
     */
    void GradeDown(float multiplicadorDaPerda = 1.0f);

    std::vector<ExtraPointItem*> mExtraPoints;

    void LoadEndScreen(); // Cria o texto escondido

    // --- Variáveis de Final de Jogo ---
    bool mIsEnding = false;
    float mEndTimer = 0.0f;
    bool mIsApproved = false; // Para saber pra onde ir ou que cor usar

    Actor* mEndTextActor = nullptr; // O texto gigante

    SoundHandle mMusicHandle;

    // --- Tela de Carregamento (pré-aquecimento dos pools de projétil) ---
    // Battle::Load() NÃO pode pré-aquecer tudo de uma vez de forma síncrona
    // sem travar o jogo por alguns segundos (carregar textura do disco para
    // 300 instâncias x N tipos de projétil é um trabalho real). Em vez
    // disso, processamos UM TIPO de projétil inteiro por frame, desenhando
    // uma barra/texto de progresso nesse meio tempo. Enquanto mIsLoading for
    // true, OnUpdate() não roda a lógica normal de gameplay.
    bool mIsLoading = false;
    std::vector<std::string> mLoadingPendingFactoryNames; // nomes ainda não pré-aquecidos
    int mLoadingTotalTypes = 0;   // quantos tipos existem no total (para a barra de progresso)

    // A TELA DE CARREGAMENTO cobre a batalha inteira - um veu escuro com o nome
    // da materia e uma barra enchendo - ate os pools estarem prontos e a barra
    // ter mostrado isso (Carregamento.h). So quando o veu some a luta comeca:
    // o chefe ataca, os alunos andam e a musica toca.
    Carregamento::Estado mCarregamento;
    Actor* mVeuDoCarregamento = nullptr;
    Actor* mBarraDoCarregamento = nullptr;
    std::vector<Actor*> mTextosDoCarregamento;
    void CriarTelaDeCarregamento();

    void StartLoadingProjectilePools();
    void UpdateLoadingStep(float deltaTime); // processa 1 tipo de projétil por chamada
    void FinishLoadingProjectilePools();

    // ---- A PAUSA, so no computador (Pausa.h) --------------------------------
    //
    // O ESC congela a cena (Scene::mAtoresCongelados) e o OnUpdate para antes
    // dos projeteis, das colisoes e dos relogios: a batalha inteira espera, e
    // volta do quadro em que parou. O menu e um veu por cima de tudo, com as
    // opcoes no meio.

    void LoadPausa();
    void AbrirPausa();
    void FecharPausa();
    void MostrarPausa(bool visivel);
    void PosicionarSelecaoDaPausa();
    void ProcessarPausa(const Uint8* keyState, bool escNovo);
    void EscolherNaPausa();

    /**
     * @brief Sai da batalha no meio, para 'destino', SEM REGISTRAR A NOTA.
     *
     * Quem sai pelo menu desistiu da tentativa, e uma tentativa abandonada nao
     * e uma nota: gravar o 40 do comeco como "ultima nota" mentiria na ficha.
     * A limpeza e a mesma do fim da batalha - os projeteis voltam aos pools e
     * o que o chefe desenhou no campo some -, so que sem a tela de nota.
     */
    void SairDaBatalha(SceneType destino);

    Pausa::Menu mPausa;

    /// Comecam apertadas: um ESC ou um botao que ja vinha segurado de antes nao
    /// conta como batida nova.
    bool mEscAnterior = true;
    bool mCimaNaPausa = true;
    bool mBaixoNaPausa = true;
    bool mConfirmarNaPausa = true;

    Actor* mVeuDaPausa = nullptr;
    Actor* mPlacaDaPausa = nullptr;
    Actor* mTituloDaPausa = nullptr;
    Actor* mSelecaoDaPausa = nullptr;
    Actor* mRodapeDaPausa = nullptr;
    std::vector<Actor*> mOpcoesDaPausa;

    /// POR CIMA DE TUDO: a faixa da nota vai ate 304 e o texto do fim usa 300.
    static constexpr int kOrdemDaPausa = 900;
    static constexpr int kLarguraDaPlacaDaPausa = 620;
    static constexpr int kAlturaDaPlacaDaPausa = 380;
    static constexpr int kLarguraDaSelecaoDaPausa = 440;
    static constexpr int kAlturaDaSelecaoDaPausa = 54;
    static constexpr float kPassoDaPausa = 66.0f;
};


#endif //DPIBULLETHELLRE_BATTLE_H
