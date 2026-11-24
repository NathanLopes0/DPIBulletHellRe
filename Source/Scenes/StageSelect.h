//
// Created by nslop on 14/10/2025.
//

#pragma once

#include "Scene.h"
#include "SelosDeMencao.h"
#include "../Entrada/Borda.h"
#include "../Motor/Game.h" // Incluir para o enum int
#include <vector>
#include "../Entrada/Navegacao.h"
#include <SDL_pixels.h>
#include <memory>
#include "../Aluno/Exportacao.h"
#include "../Curso/MapaDaGrade.h"
#include "PainelDeMenu.h"
#include "Battle/LeitorDoMenu.h"

class DrawAnimatedComponent;
class DrawCaixaComponent;
class DrawLinhasComponent;

class Font;
class StageSelectButton;
class Actor;

class StageSelect : public Scene {
public:
    explicit StageSelect(Game* game);

    ~StageSelect() override = default;

    // Funções do contrato da Scene.
    void Load() override;
    void OnProcessInput(const Uint8* keyState) override;

    void OnUpdate(float deltaTime) override;

private:

    // ---- Funções Privadas para Organização Lógica ----
    void CreateStageButtons();
    void CreateStaticUI();

    /// A ordem de desenho e crescente: o menor fica por baixo.
    static constexpr int kOrdemDasLigacoes = 60;

    /// Do centro do losango ate a ponta. Ele tem 128 de largura.
    static constexpr float kMetadeDoLosango = 64.0f;

    static constexpr float kGrossuraDaLigacao = 2.0f;

    /// As linhas entre colunas: azul frio e discreto. Elas sao cenario, e nao
    /// podem competir com os losangos nem com o texto.
    static inline const SDL_Color kCorDaLigacao{72, 92, 140, 200};

    /// O tronco entre duas colunas, com um ramo para cada materia dos dois lados. Chamada depois de
    /// CreateStageButtons: le a posicao dos botoes.
    void CriarLigacoes();

    /// Um segmento de linha, como caixa preenchida sem moldura, com o brilho por baixo. 'trecho'
    /// diz a que tronco e a que botao ele pertence (MapaDaGrade::Trecho).
    void Linha(float x, float y, float largura, float altura, MapaDaGrade::Trecho trecho);

    /// Cada segmento das ligacoes e quanto ele esta aceso (0 a 1). Acende o caminho do foco
    /// (MapaDaGrade::Caminho) e apaga o resto, devagar.
    struct Ligacao {
        DrawCaixaComponent* linha = nullptr;
        DrawCaixaComponent* brilho = nullptr;
        DrawCaixaComponent* halo = nullptr;
        /// Os segundos desde que acendeu (MapaDaGrade::Luz); negativo enquanto espera a vez.
        float relogio = kApagada;
    };
    static constexpr float kApagada = 1000.0f;
    std::vector<Ligacao> mLigacoes;
    std::vector<MapaDaGrade::Trecho> mTrechos;
    void AcenderOCaminho(int anterior, int novo);
    void AtualizarAsLigacoes(float deltaTime);

    /// A luz: no pico, a linha fica quase branca, e o brilho em volta e um ciano eletrico saturado,
    /// em duas camadas (o brilho e o halo, mais largo e mais fraco).
    static inline const SDL_Color kCorAcesa{0, 236, 255, 255};
    static inline const SDL_Color kNucleoAceso{235, 255, 255, 255};
    static constexpr float kGrossuraDoBrilho = 6.0f;
    static constexpr float kGrossuraDoHalo = 14.0f;
    static constexpr Uint8 kAlfaDoBrilho = 210;
    static constexpr Uint8 kAlfaDoHalo = 90;

    void HandleSelectionInput(const Uint8* keyState);
    size_t mSelectedIndex{};

    // ---- Funções Auxiliares de CreateStageButtons ----
    void CreateButton(const std::string &text, int subject, const Vector2 &position);
    /// Para onde a seta leva. A regra esta em Navegacao; aqui so entra a tecla.
    [[nodiscard]] size_t HandleSelectedChange(const Uint8 *keyState, size_t currSelected) const;

    // ---- Constantes de Design ----
    static constexpr int RESERVA_DE_BOTOES = 16;
    static constexpr float INPUT_DELAY = 0.2f; // Exemplo de constante para o timer

    /// Onde a grade de materias comeca e termina, em fracao da altura.
    static constexpr float kTopoDaGrade = 0.19f;
    static constexpr float kFundoDaGrade = 0.78f;
    /// O fundo da grade nas paginas das optativas: acima do cartao da materia.
    static constexpr float kFundoDasOptativas = 0.56f;

    /// Quanto o nome de uma materia fechada fica abaixo do centro do losango.
    /// O losango tem 64 de altura, entao 44 deixa o nome logo embaixo dele.
    static constexpr float kNomeAbaixoDoCadeado = 44.0f;

    // ---- Membros de Propriedade (Ownership) ----
    std::unique_ptr<Font> mStageSelectFont{};

    // ---- Membros de Estado e Observadores ----
    int mSelectedSubject{};
    float mInputTimer{}; // Timer pra mudar de botão selecionado

    Borda mBordaRanking{true};

    // Vetor de ponteiros observadores para fácil acesso aos botões. A memória real é gerenciada
    // pelo vetor mActors.
    std::vector<StageSelectButton*> mButtonObservers;

    /// A forma da grade, montada em CreateStageButtons junto com os botoes. mGrade[coluna][linha] =
    /// indice em mButtonObservers.
    Navegacao::Grade mGrade;

    // ---- O cartao da materia em foco ----

    static constexpr float kCartaoX = 268.0f;
    static constexpr float kCartaoY = 622.0f;
    static constexpr float kCartaoLargura = 400.0f;
    static constexpr float kCartaoAltura = 242.0f;
    static constexpr int kOrdemDoCartao = 70;

    static inline const SDL_Color kCorDaMolduraDoCartao{96, 118, 170, 220};
    static inline const SDL_Color kCorDoFundoDoCartao{10, 14, 30, 215};

    void UpdateStageInfo() const;

    // ---- O cartao da materia secreta: a cara de fliperama ----
    /// Com o cursor na secreta (aberta), o cartao vira outro: a moldura em neon que troca de cor, as
    /// linhas de varredura de uma tela de tubo, um glitch de vez em quando, os cantos de mira, as
    /// faiscas de pixel e o titulo com aberracao cromatica - e as linhas falam como jogo: CHEFE
    /// SECRETO, HI-SCORE, NOVO DESAFIANTE. Algo diferente vai acontecer.
    [[nodiscard]] bool CartaoGamer() const;
    void AtualizarCartaoGamer(float deltaTime);
    DrawCaixaComponent* mMolduraDoCartao = nullptr;
    DrawLinhasComponent* mNeonDoCartao = nullptr;
    Actor* mSombraVermelha = nullptr;
    Actor* mSombraCiano = nullptr;
    Actor* mSeloSecreto = nullptr;
    /// A medalha do 100 no Dificil, no canto de cima a esquerda do cartao: so quando o aluno tirou a
    /// nota cheia na materia no Dificil (Progresso::CemNoDificil). Pixel art de
    /// Ferramentas/gerar_interface.py (DPIBHMedalha), com um reflexo que passa de vez em quando.
    DrawAnimatedComponent* mMedalha = nullptr;
    static constexpr int kOrdemDaMedalha = 260;
    float mTempoGamer = 0.0f;
    bool mEraGamer = false;
    static constexpr int kOrdemDoNeon = 72;

    // ---- A compra das optativas com horas (Horas::PodeComprar) ----
    /// Todos os que ainda nao tem a optativa em foco podem compra-la agora?
    [[nodiscard]] bool DaParaComprar() const;
    [[nodiscard]] bool DaParaComprar(int materia) const;

    // ---- A matricula na optativa: o painel ao lado do cartao ----
    /// Com o cursor numa optativa fechada com preco: o que ela pede, item por item, cumprido (verde)
    /// ou nao (rosa); as horas do aluno em blocos, um por hora do preco; o botao de comprar, que acende
    /// quando da e pede a confirmacao; e o premio, o orientador novo.
    void CriarMatricula();
    void AtualizarMatricula() const;
    void MostrarMatricula(bool mostrar) const;
    /// O rodape com a acao do botao 1 da vez: "jogar", "comprar", "confirmar a compra".
    void AtualizarRodape(const std::string& acao) const;
    static constexpr float kMatriculaX = 700.0f;
    static constexpr float kMatriculaY = 616.0f;
    static constexpr float kMatriculaLargura = 400.0f;
    static constexpr float kMatriculaAltura = 266.0f;
    static constexpr int kItensDaMatricula = 5;
    static constexpr int kBlocosDaMatricula = 20;
    std::vector<Actor*> mMatricula;
    DrawCaixaComponent* mMolduraDaMatricula = nullptr;
    DrawCaixaComponent* mAbaDaMatricula = nullptr;
    Actor* mTituloDaMatricula = nullptr;
    std::vector<DrawCaixaComponent*> mMarcas;
    std::vector<Actor*> mItens;
    std::vector<DrawCaixaComponent*> mBlocos;
    Actor* mHorasDaMatricula = nullptr;
    DrawCaixaComponent* mBotaoDaMatricula = nullptr;
    Actor* mTextoDoBotao = nullptr;
    Actor* mPremioDaMatricula = nullptr;
    /// O selo "A VENDA" em cima do losango da optativa que ja pode ser comprada, piscando.
    std::vector<Actor*> mAVenda;
    float mPulso = 0.0f;
    /// A faixa de "matriculado" depois da compra, e ate quando ela fica.
    Actor* mFaixaDaCompra = nullptr;
    float mFaixaAte = -1.0f;
    /// A festa da matricula (FestaDaMatricula.h): as fichas douradas saltam dos blocos de horas ate o
    /// losango da optativa comprada, e ele da dois pulinhos com o anel e as faiscas.
    /// 'soOCarimbo': sem as fichas - so o pulinho, o anel e as faiscas (o fim da revelacao).
    void ComecarAFesta(int materia, bool soOCarimbo = false);
    void AtualizarAFesta(float deltaTime);
    DrawLinhasComponent* mFesta = nullptr;
    StageSelectButton* mBotaoDaFesta = nullptr;
    size_t mNoDaFesta = 0;
    float mTempoDaFesta = -1.0f;
    int mFichasDaFesta = 0;
    bool mCarimbou = false;
    static constexpr int kOrdemDaFesta = 260;

    // ---- A revelacao da materia secreta (Curso/Revelacao.h) ----
    // No dia em que ela aparece, a tela para tudo: escurece, a carga gira ate o meio, estoura, e o
    // losango dela nasce grande, com os parabens; o botao 1 o manda para o lugar dele na grade.
    void ComecarARevelacao(int materia);
    void AtualizarARevelacao(float deltaTime);
    void TerminarARevelacao();
    /// Le o botao durante a revelacao; o resto da tela nao anda.
    void LerARevelacao(const Uint8* keyState);
    [[nodiscard]] bool Revelando() const { return mMateriaRevelada >= 0; }
    int mMateriaRevelada = -1;
    float mTempoDaRevelacao = 0.0f;
    float mDesdeOBotao = -1.0f;
    StageSelectButton* mBotaoRevelado = nullptr;
    Actor* mLosangoGrande = nullptr;
    DrawLinhasComponent* mFundoDaRevelacao = nullptr;   ///< o veu, a carga e as ondas
    DrawLinhasComponent* mFrenteDaRevelacao = nullptr;  ///< o halo, as faiscas e o clarao
    Actor* mTituloDaRevelacao = nullptr;
    Actor* mAvisoDaRevelacao = nullptr;
    std::vector<Actor*> mNomeDaRevelacao;               ///< o nome, o professor e os parabens
    bool mSoouACarga = false;
    bool mSoouOEstouro = false;
    Borda mBordaDaRevelacao{true};
    static constexpr int kOrdemDaRevelacao = 400;
    /// A frase do cartao de uma optativa fechada com preco: quanto custa, quanto falta, ou o pedido
    /// de confirmacao.
    [[nodiscard]] std::string FraseDaCompra() const;
    /// A materia em que o botao 1 ja foi apertado uma vez: o segundo toque compra. Mudar o cursor
    /// desfaz, e ninguem gasta horas sem querer.
    int mConfirmandoCompra = -1;

    /// Cria uma das linhas do cartao e devolve o observador dela.
    Actor* TextoDoCartao(float y, int tamanho, const Vector3& cor);

    /// Troca texto e cor de uma linha do cartao.
    static void EscreverNoCartao(Actor* ator, const std::string& texto, const Vector3& cor);

    Actor* mCodigoAtor = nullptr;
    Actor* mNomeAtor = nullptr;
    Actor* mProfessorAtor = nullptr;
    Actor* mEstadoAtor = nullptr;
    /// O rival da turma (Rival.h): quem esta logo acima no ranking da materia, no modo e nivel da vez.
    Actor* mRivalAtor = nullptr;
    /// As fichas da turma, lidas uma vez ao abrir a tela.
    std::vector<Exportacao::FichaDeAluno> mFichas;
    /// As mencoes da materia em foco (Mencoes.h), na ultima linha do cartao: o que ainda ha para
    /// buscar nela.
    std::unique_ptr<SelosDeMencao> mMencoes;

    // ---- A prova da semana ----

    /// O painel da prova da semana (ProvaDaSemana.h), no canto de baixo a direita: a materia, quanto
    /// falta, o podio e o lugar de quem esta jogando - e a marca em cima do botao da materia.
    void CriarProvaDaSemana();
    /// O cartao da prova da semana, menor que o da materia: so aparece com o cursor nela.
    static constexpr float kProvaX = 1046.0f;
    static constexpr float kProvaY = 652.0f;
    static constexpr float kProvaLargura = 272.0f;
    static constexpr float kProvaAltura = 176.0f;
    int mMateriaDaSemana = -1;
    std::vector<Actor*> mCartaoDaProva;
    void MostrarCartaoDaProva(bool mostrar);

    // ---- O mapa: as obrigatorias na primeira pagina, as optativas nas seguintes (MapaDaGrade.h) ----
    /// Um ator da grade e onde ele fica com a camera na primeira pagina.
    struct NoDoMapa {
        Actor* ator = nullptr;
        Vector2 base;
    };
    std::vector<NoDoMapa> mNosDoMapa;
    void NoMapa(Actor* ator);
    std::vector<MapaDaGrade::Coluna> mColunas;
    int mPaginas = 1;
    int mPagina = 0;
    float mCamera = 0.0f;
    /// A pagina da coluna do botao em foco, e a camera vai ate ela.
    void SeguirOFoco(bool deUmaVez);
    void AtualizarDicas();
    Actor* mTituloAtor = nullptr;
    Actor* mDicaDireita = nullptr;
    Actor* mDicaEsquerda = nullptr;

    /// Quanto uma linha de aluno fica abaixo da anterior, quando ha dois. O
    /// rotulo tem 44 de altura, entao 38 os encosta sem os sobrepor.
    static constexpr float kEntreOsAlunos = 38.0f;

    /// Quem esta jogando, e o aviso de que visitante nao salva. Uma linha por
    /// posicao do painel.
    void CriarIdentificacaoNaTela();
    Actor* mAlunoAtor = nullptr;
    Actor* mTrocarAtor = nullptr;

    /// Com a borda em false, voltar do ranking com o botao ainda apertado jogava o aluno direto
    /// para a tela de matricula.
    Borda mBordaTrocar{true};
    /// O START (no computador, o ESC) pergunta se o aluno quer voltar ao menu principal: o painel,
    /// com o foco no "nao", e o Botao 2 ou o START de novo fecham sem sair.
    Borda mBordaDoStart{true};
    Borda mBordaDoNao{true};
    std::unique_ptr<PainelDeMenu> mPainelDeSaida;
    std::unique_ptr<LeitorDoMenu> mLeitorDaSaida;
    bool mPerguntandoSaida = false;
    int mOpcaoDeSaida = 0;   ///< 0: ficar; 1: voltar ao menu
    void AbrirASaida();
    void FecharASaida();
    void LerASaida(const Uint8* keyState, bool startNovo);
    /// O Botao 4: a troca de aluno(s) (TrocaDeAlunos).
    Borda mBordaTrocarAlunos{true};

    /// F9, para quem desenvolve: desfaz as matriculas nas optativas (Game::DesfazerMatriculas), para
    /// ver a compra e a festa de novo.
    Borda mBordaDesfazer{true};

    /// F10, para quem desenvolve: abre as materias secretas sem a regra (Game::LiberarSecretas).
    Borda mBordaLiberar{true};

    /// Borda do enter, e comeca em true. A cena anterior (identificacao) termina com o enter
    /// apertado - e ele que manda para ca.
    Borda mBordaEntrar{true};
};