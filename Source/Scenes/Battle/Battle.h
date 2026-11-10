//
// Created by nslop on 16/09/2024.
//

#ifndef DPIBULLETHELLRE_BATTLE_H
#define DPIBULLETHELLRE_BATTLE_H

#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include "../Scene.h"
#include "../../Regras/Intercambio.h"
#include "../../Regras/Papeis.h"
#include "../../Regras/Cobertura.h"
#include "../../Motor/Game.h"
#include "../../Motor/AudioSystem.h"
#include "../../Interface/Pausa.h"
#include "../../Entrada/Borda.h"
#include "../../Interface/Resultado.h"
#include "../../Interface/Devolutiva.h"
#include "../../Regras/Dificuldade.h"
#include "../../Interface/Hud.h"
#include "../../Regras/Variacao.h"
#include "OuvinteDaBatalha.h"
#include "../../Actors/Teachers/BossFactory/IBossFactory.h"

class Font;
class Player;
class ProjectileManager;
class BattleHUD;
class ExtraPointItem;
class Orientador;
class PainelDeMenu;
class BossProjectile;
class PlayerProjectile;

class Battle : public Scene {

public:

    explicit Battle(Game* game, int selectedStage);
    ~Battle() override;
    void Load() override;
    void VerificarColisoes();
    /// Uma fase do chefe acabou e outra comecou. Chamado pelo callback que o Boss registra na
    /// maquina de estados.
    void AoMudarDeFase(float novaDuracao);

    [[nodiscard]] Boss* Chefe() const { return mChefe; }

    /// O orientador do TCC, quando ha um em campo.
    [[nodiscard]] Orientador* OrientadorEmCampo() const { return mOrientador; }

    /// Paga uma conquista da dupla: um ganho bruto, inteiro (nao passa por Nota::PorJogador - ver
    /// SubirNota).
    void Conquistar(const float bruto) { if (EmCombate()) SubirNota(bruto); }

    /// Se a luta esta valendo: nem carregando os projeteis, nem na tela de fim.
    /// Quem age por conta propria (o orientador) so age enquanto isto vale.
    [[nodiscard]] bool EmCombate() const { return !mCarregando && !mAcabando; }

    /// Todos os corpos do chefe em campo, e nao so o original. Normalmente e um so.
    [[nodiscard]] const std::vector<Boss*>& Corpos() const { return mCorpos; }

    /// O modo desta batalha (Dificuldade.h).
    [[nodiscard]] Dificuldade::Nivel NivelDeDificuldade() const { return mNivel; }

    /// A semente desta luta (Variacao.h): de onde cada chefe tira o que muda de uma batalha para a
    /// outra dentro de cada fase.
    [[nodiscard]] std::uint32_t SementeDaBatalha() const { return mSemente; }

    /// Poe um segundo corpo em campo, com os ataques da fase nomeada. Chamada pelo chefe ao entrar
    /// numa fase que declara "clone" em fases.json.
    void DividirChefe(const std::string& faseDoClone);

    /// Tira de campo os corpos extras, deixando so o original.
    void JuntarCorpos();

    /// Os jogadores em campo, na ordem das posicoes do painel.
    [[nodiscard]] const std::vector<Player*>& Jogadores() const { return mJogadores; }

    /// Em quem um ataque mirado mira.
    [[nodiscard]] Player* Alvo() const;

    /// Em qual posicao esta a vez: a de quem mais incomodou o professor (Papeis.h) ou, se os dois
    /// incomodam parecido, a do revezamento. A vez dada (DarAVez) vale para o proximo disparo.
    [[nodiscard]] size_t IndiceDoAlvo() const {
        return mVezDada ? mVezDoAlvo : mIncomodo.Alvo(mVezDoAlvo, mJogadores.size());
    }

    /// Em quem a mira esta presa (o marcador vermelho sobre o aluno); -1 quando os dois revezam.
    [[nodiscard]] int FocoDaMira() const { return mIncomodo.Foco(mJogadores.size()); }

    /// O jogador de uma posicao, ou nullptr fora da lista. E como um projetil
    /// resolve o numero que guardou.
    [[nodiscard]] Player* JogadorEm(size_t posicao) const;

    /// Passa a vez para o proximo jogador. Chamado uma vez por disparo, em Boss::ExecuteAttack - o
    /// unico lugar por onde todo ataque de todo chefe passa.
    void RevezarAlvo();

    /// Da a vez a um jogador, antes de um disparo que e dele - mesmo que a mira esteja no outro.
    void DarAVez(size_t posicao);
    [[nodiscard]] ProjectileManager* GerenteDeProjeteis() const { return mProjeteis.get(); }

    /// A fonte do placar (Carlito), para quem escreve codigo no campo (EtiquetaDeCodigo). E da
    /// batalha, que vive mais que os atores dela.
    [[nodiscard]] Font* FonteDoPlacar() const { return mFonteDoPlacar.get(); }

    SDL_FRect GetPlayfieldBounds() const override;
    /// Um aluno gastou um Ponto Extra. Recebe quem gastou porque em dupla o efeito e no outro: a
    /// tela nao e limpa, o parceiro fica imune.
    void AoGastarPontoExtra(Player* quemGastou);

    /// O outro aluno em campo, ou nullptr se este estiver sozinho.
    [[nodiscard]] Player* OutroJogador(const Player* quem) const;
    void SoltarPontoExtra(Vector2 position);

    void EncerrarBatalha(bool aprovado);

    /// A nota da batalha em curso. Quem precisa dela e a maquina de estados do chefe, para decidir
    /// entre a repescagem e o fim.
    [[nodiscard]] float NotaAtual() const { return mNota; }
    /// Quantas vezes a dupla foi acertada nesta batalha.
    [[nodiscard]] int AcertosSofridos() const { return mAcertosSofridos; }
    /// O indice da materia desta batalha (Materias::Lista).
    [[nodiscard]] int MateriaDaBatalha() const { return mMateria; }

    /// A janela perdeu o foco: a luta pausa sozinha (Pausa::PausaAoPerderOFoco).
    void AoPerderOFoco() override;

protected:
    void AtualizarBarraDeTempo() const;

    void AtualizarTextoDaNota();
    void AtualizarBarraDaNota();

    void OnUpdate(float deltaTime) override;
    void OnProcessInput(const Uint8 *keyState) override;

private:
    // ---- As regras de colisao, na ordem em que VerificarColisoes as aplica ----
    /// O abrigo come os projeteis do chefe.
    void ColisoesDoAbrigo(const std::vector<std::unique_ptr<BossProjectile>>& bossProjectiles);
    /// O escudo do Ponto Extra come os projeteis do chefe.
    void ColisoesDoEscudo(const std::vector<std::unique_ptr<BossProjectile>>& bossProjectiles);
    /// Os projeteis do chefe acertam os alunos.
    void ColisoesTiroDoChefeNoAluno(const std::vector<std::unique_ptr<BossProjectile>>& bossProjectiles);
    /// O tiro do aluno derruba (ou murcha em) projeteis do chefe.
    void ColisoesTiroDoAlunoNosProjeteis(const std::vector<std::unique_ptr<PlayerProjectile>>& playerProjectiles, const std::vector<std::unique_ptr<BossProjectile>>& bossProjectiles);
    /// O tiro do aluno acerta o chefe.
    void ColisoesTiroDoAlunoNoChefe(const std::vector<std::unique_ptr<PlayerProjectile>>& playerProjectiles);
    /// O aluno encosta no chefe.
    void ColisoesDeContato();
    /// O aluno pega um Ponto Extra.
    void ColisoesComPontoExtra();

    // ---- Funções Auxiliares de Lógica Interna ----
    void CriarHUD();
    void CriarJogadores();
    void CriarChefe();

    /// O orientador, se a materia pede um e alguem foi escolhido. Quem escolhe e a
    /// EscolhaDoOrientador, e o escolhido chega pelo Game (Game::Orientador).
    void CriarOrientador();

    void CriarRelogioDaFase();

    void CriarBarraDeTempo();
    void CriarBarraDaNota();

    // ---- O placar: o que o chefe anuncia na fase ("anuncio" em fases.json) ----
    void CriarPlacar();
    void AtualizarPlacar();

    // ---- O intercambio: os alunos trocam de lugar ("intercambio" em fases.json) ----
    void CriarIntercambio();
    void AtualizarIntercambio(float deltaTime);

    // ---- Membros de Estado da Batalha ----
    float mNota;
    int mMateria{};

    /// Acertos sofridos na fase atual.
    int mAcertosNaFase{};

    /// Fecha a fase em curso: paga o bonus se ela foi limpa e zera o contador.
    void EncerrarFase();

    /// O callback da maquina de estados dispara tambem ao entrar na primeira fase, quando nao ha
    /// fase anterior.
    bool mPrimeiraFase = true;

    /// Quantas fases pagaram o bonus de fase limpa. Para o resumo do fim
    /// (Resultado.h); o registro da tentativa conta as dele pelo AoLimparFase.
    int mFasesLimpas = 0;

    /// Quanto tempo o aviso de fase limpa ainda fica na tela.
    float mAvisoDeFaseLimpa{};

    Actor* mFaseLimpaAtor = nullptr;

    Dificuldade::Nivel mNivel = Dificuldade::Nivel::Normal;

    std::uint32_t mSemente = 0;

    // ---- Quem ouve a batalha (OuvinteDaBatalha) ----

    std::vector<std::unique_ptr<OuvinteDaBatalha>> mOuvintes;

    /// Anuncia a todos os ouvintes, na ordem em que entraram:
    /// Avisar([&](OuvinteDaBatalha& o) { o.AoLimparFase(*this); }).
    template <typename Aviso>
    void Avisar(Aviso&& aviso) {
        for (const auto& o : mOuvintes) aviso(*o);
    }

    /// O selo Dificil, na ponta direita da faixa da nota.
    Actor* mSeloDoNivel = nullptr;

    /// Quantos projeteis do chefe acertaram o jogador nesta batalha. Zera a cada batalha junto com
    /// a nota: o teto de Nota e por batalha, nao por perfil.
    int mAcertosSofridos{};

    // ---- A faixa da nota, na base da tela ----

    /// A altura da barra.
    static constexpr int kBarraAltura = 28;

    /// A sobra dos lados. A barra ia de ponta a ponta da tela e encostava nas
    /// bordas, o que a fazia parecer um risco no monitor em vez de uma peca.
    static constexpr int kBarraMargemLateral = 40;

    /// Do fundo da tela ate o topo da barra.
    static constexpr int kBarraDoFundo = 54;

    /// A placa atras de um texto da faixa.

    // ---- O Ponto Extra do co-op, a vista (PontoExtra.h): o feixe de quem gastou ----
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

    // ---- A moldura do campo e os efeitos (Hud.h) ----
    class DrawMolduraDoCampoComponent* mMoldura = nullptr;
    Hud::Caixa mAreaDoAluno;
    void CriarMoldura();
    /// O fundo do campo (FundoDoCampo.h): nao e preto liso.
    void CriarFundo();
    void AtualizarMoldura();

public:
    /// O socket do Vitor (o poder dele, como orientador): por 'segundos', as paredes dos lados
    /// viram rede.
    void AbrirRede(float segundos) { mRede = std::max(mRede, segundos); }
    [[nodiscard]] bool RedeAberta() const { return mRede > 0.0f; }
    /// O aluno passou pela rede: o clarao dos dois lados.
    void Atravessou(const Vector2& de, const Vector2& para);

private:
    float mRede = 0.0f;
    /// O "prof" escrito embaixo do brilho de cada corpo do professor.
    std::vector<Actor*> mRotulosDoProfessor;

    struct FaiscaEmCampo {
        Vector2 onde;
        Hud::Faisca tipo = Hud::Faisca::Acerto;
        float idade = 0.0f;
        int quem = -1;   ///< o aluno (a cor do "!" da Resposta); -1 = ninguem
    };
    std::vector<FaiscaEmCampo> mFaiscas;
    class DrawLinhasComponent* mEfeitos = nullptr;
    /// Um efeito em 'onde'. Com muitos ao mesmo tempo, os mais velhos saem.
    void Faiscar(const Vector2& onde, Hud::Faisca tipo, int quem = -1);

    /// O tiro de um aluno derrubou um projetil: se ele ia no parceiro, e uma cobertura (Cobertura.h),
    /// paga inteira, com a faisca dela. Devolve se foi.
    bool Cobrir(const PlayerProjectile& tiro, const BossProjectile& projetil);

    /// O aluno para as contas da cobertura: onde ele esta, o colisor e se algo o atinge.
    [[nodiscard]] Cobertura::Aluno AlunoDaCobertura(const Player* aluno) const;

    /// Na dupla, o projetil destrutivel que vai num aluno brilha na cor dele (Cobertura::AlunoNoCaminho):
    /// quem esta embaixo sabe que derruba-lo e a cobertura. E a mesma conta de Cobrir.
    void MarcarOCaminhoDosAlunos();

    /// O projetil passou rente ao aluno sem acerta-lo (Raspao.h): paga o raspao, uma vez por projetil.
    void Raspar(Player& jogador, const class CircleColliderComponent& colisor, class BossProjectile& projetil,
                const class CircleColliderComponent& colisorDoProjetil);
    /// A posicao (0 ou 1) do aluno 'ator', ou -1 se ele nao e um aluno.
    [[nodiscard]] int PosicaoDoAluno(const Actor* ator) const;
    void AtualizarEfeitos(float deltaTime);

    // ---- O Ponto Extra de solo: a onda e as moedas ----
    struct Onda {
        Vector2 centro;
        float t = 0.0f;
    };
    std::vector<Onda> mOndas;
    struct Moeda {
        Vector2 de;
        float t = 0.0f;
    };
    std::vector<Moeda> mMoedas;
    void AtualizarOndas(float deltaTime);
    [[nodiscard]] Vector2 PontaDaBarra() const;
    static constexpr size_t kMaximoDeMoedas = 40;
    static constexpr float kVooDaMoeda = 0.55f;
    static constexpr int kOrdemDosEfeitos = 104;
    static constexpr size_t kMaximoDeFaiscas = 64;
    /// A moldura e fundo, atras de tudo - ate do quadro (60).
    static constexpr int kOrdemDaMoldura = 40;
    static constexpr int kOrdemDoFundo = 10;

    static constexpr int kPlacaLargura = 112;

    /// A dos avisos e mais larga: "TESTE FINAL" tem onze letras contra as seis
    /// de "100.00".
    static constexpr int kPlacaDoAvisoLargura = 196;

    /// A mesma altura da barra, e nao um numero proprio: dois pixels de diferenca ja faziam a placa
    /// aparecer por cima da moldura.
    static constexpr int kPlacaAltura  = kBarraAltura;
    static constexpr int kOrdemDaPlaca = 303;   ///< o texto da faixa usa 304
    static inline const SDL_Color kSemMoldura{0, 0, 0, 0};
    static inline const SDL_Color kFundoDaPlaca{6, 8, 16, 135};
    /// A placa do numero da nota, mais fechada que a dos avisos e com um fio de moldura.
    static inline const SDL_Color kFundoDaPlacaDaNota{6, 8, 16, 200};
    static inline const SDL_Color kMolduraDaPlacaDaNota{255, 255, 255, 55};

    // ---- Ponteiros observadores do Boss e do Player, que vão estar dentro ----
    Boss* mChefe{};

    /// O original mais os clones vivos.
    std::vector<Boss*> mCorpos;

    /// Os jogadores, em vez de um.
    std::vector<Player*> mJogadores;

    /// O orientador do TCC, ou nulo. O ator e da cena, como todos.
    Orientador* mOrientador = nullptr;

    /// De quem e a vez de ser mirado.
    size_t mVezDoAlvo = 0;
    /// A vez foi dada (DarAVez) para o proximo disparo, por cima da mira.
    bool mVezDada = false;
    /// O "v" da mira fica isto acima do centro do aluno: logo acima da cabeca (o desenho tem 64 px).
    static constexpr float kAlturaDaMira = 44.0f;
    /// O quanto cada aluno incomodou o professor: em quem os ataques mirados vao (Papeis.h).
    Papeis::Incomodo mIncomodo;

    // O gerente de projéteis: Um sistema especializado, possuído pela cena.
    std::unique_ptr<ProjectileManager> mProjeteis;

    // O HUD.
    std::unique_ptr<BattleHUD> mHUD;
    std::unique_ptr<Font> mFonteDaNota;

    /// A Carlito tem a notacao inteira, inclusive os sobrescritos e o delta.
    std::unique_ptr<Font> mFonteDoPlacar;
    Actor* mPlacarAtor = nullptr;

    /// E o texto, e nao a fase, que se compara: dentro de uma fase com Intercambio o placar troca
    /// de texto durante o aviso.
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

    /// Logo acima dos projeteis (88 a 92) e abaixo do aluno (100).
    static constexpr int kOrdemDoMarcador = 95;

    static constexpr int kPlacarLargura = 320;
    static constexpr int kPlacarAltura  = 46;
    static constexpr int kPlacarMargem  = 16;
    static constexpr int kTamanhoDoPlacar = 30;

    /// Atras dos projeteis (88 a 92) e do aluno (100), e.
    static constexpr int kOrdemDoPlacar = 60;
    Actor* mBarraDeTempoAtor = nullptr;
    Actor* mRelogioAtor = nullptr;

    Actor* mBarraDaNotaAtor = nullptr;
    Actor* mTextoDaNotaAtor = nullptr;

    /// O "TESTE FINAL" na faixa da nota. Escondido ate a nota chegar a 100, e a
    /// partir dali fica - ver AtualizarTextoDaNota.
    Actor* mTesteFinalAtor = nullptr;

    /// Soma um ganho bruto na nota da batalha. Sem valor padrao.
    void SubirNota(float amount);

    /// Quanto vale, nesta batalha, algo que um jogador causou.
    [[nodiscard]] float PorJogador(float bruto) const;

    /// O que um acerto sofrido custa: dividido E aliviado.
    [[nodiscard]] float PerdaPorJogador(float bruto) const;
    /// Cobra um acerto sofrido.
    void DescerNota(float multiplicadorDaPerda = 1.0f);

    std::vector<ExtraPointItem*> mPontosExtras;

    // ---- O fim da luta: a tela de resultado (Resultado.h) ----

    /// O painel do resultado, do tamanho do resumo: criado no fim, quando se sabe quantas linhas ele tem.
    void CriarResultado(int linhas);
    void MostrarResultado(const Resultado::Resumo& resumo);
    /// O que mais acertou o aluno, na linha da devolutiva: o desenho de cada projetil e quantas vezes.
    void MostrarCausas(const std::vector<Devolutiva::Causa>& causas, const Vector2& onde);
    /// A formatura: a personagem de capelo dos dois lados do titulo - no solo, a mesma; na dupla,
    /// cada aluno do seu lado. Nao depende de onde o aluno estava quando a luta acabou.
    void MostrarFormandos();
    /// Um acerto sofrido por 'jogador', que levou 'perda' de nota, por 'causa' (o tipo do projetil, ou
    /// o esbarrao): para a devolutiva e a justica.
    void AnotarPerda(const Player& jogador, const std::string& causa, float perda);
    void ProcessarResultado(const Uint8* keyState);
    void EscolherNoResultado();

    bool mAcabando = false;
    /// Segundos desde o fim da luta: a trava do menu e a volta sozinha no
    /// gabinete contam por ele.
    float mTempoNoFim = 0.0f;
    int mResultadoSelecionado = 0;
    std::unique_ptr<PainelDeMenu> mPainelDoResultado;

    /// A devolutiva: onde a nota caiu e por que (Devolutiva.h).
    Devolutiva::Caderno mCaderno;
    /// A justica, na dupla: o que cada aluno fez.
    std::vector<Devolutiva::Parte> mPartes;

    SoundHandle mMusica;

    // ---- Tela de Carregamento (pré-aquecimento dos pools de projétil) ----
    bool mCarregando = false;
    std::vector<std::string> mTiposACarregar; // nomes ainda não pré-aquecidos
    int mTiposNoTotal = 0;   // quantos tipos existem no total (para a barra de progresso)

    // A tela de carregamento cobre a batalha inteira - um veu escuro com o nome da materia e uma
    // barra enchendo.
    std::unique_ptr<class TelaDeCarregamento> mTelaDeCarregamento;

    void ComecarCarregamento();
    void AvancarCarregamento(float deltaTime); // processa 1 tipo de projétil por chamada
    void TerminarCarregamento();

    // ---- A pausa, so no computador (Pausa.h) ----

    void CriarPausa();
    void AbrirPausa();
    /// Fecha o menu e comeca a contagem: a luta segue congelada ate ela acabar.
    void FecharPausa();
    /// A contagem acabou: os atores e a musica voltam.
    void Retomar();
    void ProcessarPausa(const Uint8* keyState, bool escNovo);
    void EscolherNaPausa();

    /// Sai da batalha no meio, para 'destino', sem registrar a nota.
    void SairDaBatalha(SceneType destino);

    Pausa::Menu mPausa;
    std::unique_ptr<PainelDeMenu> mPainelDaPausa;

    /// A contagem da volta (Pausa::kRetomada): segundos que faltam, e 0 fora dela.
    float mRetomada = 0.0f;
    Actor* mNumeroDaRetomada = nullptr;
    [[nodiscard]] bool Retomando() const { return mRetomada > 0.0f; }

    // ---- As batidas dos menus (a pausa e o resultado) ----

    /// So o que acabou de ser apertado neste quadro, de qualquer um dos dois.
    struct BatidasDoMenu {
        bool cima = false;
        bool baixo = false;
        bool confirmar = false;
    };
    BatidasDoMenu LerMenu(const Uint8* keyState);
    void SegurarMenu() { mBordaCima.Segurar(); mBordaBaixo.Segurar(); mBordaConfirmar.Segurar(); }

    Borda mBordaEsc{true};
    Borda mBordaCima{true};
    Borda mBordaBaixo{true};
    Borda mBordaConfirmar{true};

    /// Por cima de tudo: a faixa da nota vai ate 304.
    static constexpr int kOrdemDosMenus = 900;
    /// O lado do desenho de cada projetil na devolutiva: o de uma linha do resultado.
    static constexpr float kLadoDoIcone = 34.0f;
    /// Quem se formou, dos lados do titulo: dentro da placa do resultado e acima das linhas.
    static constexpr float kAfastamentoDoFormando = 200.0f;
    static constexpr float kEscalaDoFormando = 1.5f;
    /// O resultado e mais largo que a pausa: a linha da devolutiva traz o nome da fase.
    static constexpr int kLarguraDoResultado = 760;
};

#endif // DPIBULLETHELLRE_BATTLE_H.
