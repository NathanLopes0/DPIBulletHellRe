//
// Created by nslop on 16/09/2024.
//

#ifndef DPIBULLETHELLRE_BATTLE_H
#define DPIBULLETHELLRE_BATTLE_H

#include <memory>
#include <string>
#include <vector>
#include "../Scene.h"
#include "../../Game.h"
#include "../../AudioSystem.h"
#include "../../Actors/Teachers/BossFactory/IBossFactory.h"

#define GRADE_CHANGE_UP 0.56f
#define GRADE_CHANGE_DOWN 6.f


class Font;
class Player;
class ProjectileManager;
class BattleHUD;
class ExtraPointItem;

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
    [[nodiscard]] ProjectileManager* GetProjectileManager() const { return mProjectileManager.get(); }

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

    void LoadTimeClock();

    void LoadTimeBar();
    void LoadGradeBar();


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
    /// OnPlayerUsedExtraPoint, ao lado de GRADE_CHANGE_UP e GRADE_CHANGE_DOWN
    /// que ja moravam em constantes - e agora ele e repartido como os outros,
    /// entao vale ter nome.
    static constexpr float kGanhoDoPontoExtra = 10.0f;

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

    // --- Ponteiros observadores do Boss e do Player, que vão estar dentro
    // do vetor mActors da própria cena Battle. Ideal para, mesmo num vetor de Actors,
    // chamar funções específicas de Player e de Boss, e também para acesso rápido sem percorrer o vetor.
    Boss* mBoss{};

    /// OS JOGADORES, em vez de um. Ver Jogadores() e Alvo().
    std::vector<Player*> mJogadores;

    /// De quem e a vez de ser mirado. Ver RevezarAlvo.
    size_t mVezDoAlvo = 0;

    // O GERENTE DE PROJÉTEIS: Um sistema especializado, possuído pela cena
    std::unique_ptr<ProjectileManager> mProjectileManager;

    // O HUD
    std::unique_ptr<BattleHUD> mHUD;
    std::unique_ptr<Font> mGradeBarFont;
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
    void GradeDown();

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
    Actor* mLoadingTextActor = nullptr;

    void StartLoadingProjectilePools();
    void UpdateLoadingStep(); // processa 1 tipo de projétil por chamada
    void FinishLoadingProjectilePools();
};


#endif //DPIBULLETHELLRE_BATTLE_H
