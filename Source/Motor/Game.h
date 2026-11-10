//
// Created by nslop on 03/04/2024.
//

#pragma once

#include <map>
#include <string>
#include <memory>
#include <vector>
#include <SDL.h>
#include "Math.h"
#include "../Scenes/Scene.h"
#include "../Personagem/Personagens.h"
#include "../Aluno/Progresso.h"
#include "Gabinete.h"
#include "../Pesquisa/Experimento.h"
#include "../Simulacao/Simulacao.h"
#include "../Entrada/Painel.h"
#include "../Aluno/Sessao.h"
#include "Configuracao.h"
#include "../Regras/Dificuldade.h"

class IBossFactory;
class Actor;

class Game {
public:

    // Uma materia e o indice dela em Materias::Carregadas(). Aqui havia um enum com as dez materias
    // escritas a mao.

    Game(int windowWidth, int windowHeight, const Gabinete::Configuracao& gabinete = {},
         const Simulacao::Configuracao& simulacao = {});
    ~Game();

    bool Initialize();
    void RunLoop();
    void Shutdown();
    void Quit() { mIsGameRunning = false; }

    // Actor functions.
    void LoadInitialScene();

    // Texture.
    class SDL_Texture* LoadTexture(const std::string& texturePath);

    /// Os quadros de uma folha de sprites (o JSON ao lado do PNG), lidos uma vez por caminho e
    /// compartilhados (padrao Flyweight, como as texturas).
    const std::vector<SDL_Rect>& QuadrosDaFolha(const std::string& dataPath);

    /// A silhueta da imagem: o mesmo desenho, todo branco e opaco onde ha pixel, transparente onde
    /// nao ha (Destrutivel::AlfaDaSilhueta).
    class SDL_Texture* LoadSilhueta(const std::string& texturePath);

    /// A textura guardada sob esta chave, ou nullptr. Para texturas que nao vem de arquivo - a
    /// personagem composta e a primeira delas.
    [[nodiscard]] class SDL_Texture* TexturaGuardada(const std::string& chave) const;

    /// Guarda uma textura ja pronta. O Game passa a ser dono dela.
    void GuardarTextura(const std::string& chave, class SDL_Texture* textura);

    /// Destroi e esquece uma textura guardada. Existe para a tela de criacao de personagem, que
    /// compoe uma textura nova a cada seta apertada.
    void EsquecerTextura(const std::string& chave);

    /// Devolve (criando na primeira chamada) o xadrez magenta/preto usado como marcador de textura
    /// ausente.
    SDL_Texture* GetPlaceholderTexture();

    class SDL_Renderer* GetRenderer() { return mRenderer; }

    // Window functions.
    [[nodiscard]] int GetWindowWidth() const { return mWindowWidth; }
    [[nodiscard]] int GetWindowHeight() const { return mWindowHeight; }

    // Audio System.
    class AudioSystem *GetAudio() { return mAudio.get(); }

    // Scenes.
    [[nodiscard]] Scene* GetScene() const { return mScene.get(); }
    void RequestSceneChange(Scene::SceneType nextScene);

    [[nodiscard]] Scene::SceneType GetCurrSceneType() const;

    IBossFactory* GetFactory(size_t n);

    // Funções pra retornar o estágio escolhido em StageSelect (retorna INF213 caso não tenha sido modificado,
    // porque é oq foi definido no construtor)
    [[nodiscard]] int GetSelectedStage() const { return mSelectedStage; }
    void SetSelectedStage(const int subject) { mSelectedStage = subject; }

    /// A materia em foco na selecao de fases: a ultima que o cursor tocou nesta partida, ou -1.
    [[nodiscard]] int MateriaEmFoco() const { return mMateriaEmFoco; }
    void FocarMateria(const int materia) { mMateriaEmFoco = materia; }

    /// O orientador da proxima batalha: o indice da materia de que ele e professor, ou -1 para
    /// batalha sem orientador.
    void DefinirOrientador(const int materia) { mOrientador = materia; }
    [[nodiscard]] int Orientador() const { return mOrientador; }

    // ---- Notas ----

    /// A maior nota que o jogador ja tirou. Zero se nunca jogou.
    [[nodiscard]] float GetMelhorNota(const int materia) const {
        return ProgressoDoMapa().MelhorNota(materia);
    }

    /// O recorde do jogador 1 no modo e no nivel desta partida: o que a tela de resultado compara
    /// para dizer "NOVO RECORDE!".
    [[nodiscard]] float RecordeDaPartida(int materia) const;

    /// Como os rodapes nomeiam os botoes nesta sessao.
    [[nodiscard]] Painel::Jeito JeitoDoRodape() const {
        return mGabinete.arcade ? Painel::Jeito::SoOPainel : Painel::Jeito::ComATecla;
    }

    /// Se o jogo abriu no gabinete (--arcade). Quem precisa saber e quem muda de comportamento la -
    /// a pausa da batalha, que no gabinete nao existe (Pausa.h).
    [[nodiscard]] bool NoGabinete() const { return mGabinete.arcade; }

    /// As mecanicas que esta execucao desliga (--condicao): a condicao do experimento, gravada em
    /// cada tentativa.
    [[nodiscard]] const Experimento::Condicao& Condicao() const { return mCondicao; }

    /// O jogo rodando sozinho, para medir e fotografar (Simulacao.h): vazia, o jogo de sempre.
    [[nodiscard]] const Simulacao::Configuracao& ConfiguracaoDaSimulacao() const { return mSimulacao; }

    /// Quantos quadros ja foram desenhados. E o relogio do roteiro de teclas, das fotos e do fim
    /// forcado da simulacao.
    [[nodiscard]] int Quadro() const { return mQuadro; }

    /// O modo de dificuldade das batalhas (Dificuldade.h).
    [[nodiscard]] Dificuldade::Nivel NivelDeDificuldade() const { return mDificuldade; }
    void DefinirDificuldade(Dificuldade::Nivel nivel) { mDificuldade = nivel; }

    // ---- As opcoes da maquina (Configuracao.h) ----

    [[nodiscard]] int VolumeDaMusica() const { return mConfiguracao.volumeMusica; }
    [[nodiscard]] int VolumeDosEfeitos() const { return mConfiguracao.volumeEfeitos; }
    [[nodiscard]] bool TelaCheia() const { return mConfiguracao.telaCheia; }
    [[nodiscard]] bool MostrarHitbox() const { return mConfiguracao.mostrarHitbox; }

    void DefinirVolumeDaMusica(int volume);
    void DefinirVolumeDosEfeitos(int volume);
    void DefinirTelaCheia(bool cheia);
    void DefinirHitbox(bool mostrar);

    /// A mesma regra que o desbloqueio le, e nao uma comparacao com 60 repetida em quem desenha. Do
    /// jogador 1.
    [[nodiscard]] bool Aprovado(const int materia) const {
        return ProgressoDoMapa().Aprovado(materia);
    }

    /// Registra o resultado de uma batalha: a retomada passa a ser esta nota, e o recorde so sobe.
    /// Chamado uma vez, ao fim da batalha.
    void RegistrarNota(int subject, float nota);

    /// Passa a jogar como este aluno, carregando a ficha dele do disco. A matricula ja deve vir
    /// canonica.
    void IdentificarAluno(const std::string& matriculaCanonica);

    /// Passa a jogar sem se identificar. O progresso da sessao comeca zerado e nao e gravado.
    void JogarComoVisitante();

    // ---- A entrada, de um ou de dois ----

    /// Quantos jogam esta partida. So o menu principal decide.
    void DefinirModoDeJogo(Sessao::Modo modo);
    [[nodiscard]] size_t QuantosJogadores() const { return mSessoes.size(); }

    [[nodiscard]] size_t PosicaoEntrando() const { return mPosicaoEntrando; }

    /// As sessoes, para quem precisa olhar todas.
    [[nodiscard]] const std::vector<Sessao::Dados>& Sessoes() const { return mSessoes; }

    /// A posicao atual terminou de entrar: chama a proxima ou vai jogar. A regra mora aqui, e nao
    /// nas duas telas que terminam a entrada.
    void AvancarEntrada();

    /// O aluno desta posicao ja se formou: passou na materia da formatura (Personagens::Formatura) e
    /// usa o capelo. Vale para o visitante tambem, enquanto a sessao dura.
    [[nodiscard]] bool Formado(size_t posicao) const;

    /// Abre a criacao de personagem para quem ja entrou (da dificuldade, Botao 3): e assim que um
    /// perfil que ja existia troca de personagem. A volta e para a dificuldade.
    void EditarPersonagem(size_t posicao);
    /// A criacao foi aberta por EditarPersonagem, e nao pela entrada de um aluno novo.
    [[nodiscard]] bool EditandoPersonagem() const { return mEditandoPersonagem; }
    /// Fecha a edicao: a entrada volta a posicao em que estava, e a tela volta para a dificuldade.
    void TerminarEdicaoDePersonagem();

    /// Desiste da entrada em curso e volta a ser uma partida de um.
    void CancelarEntrada();

    /// O que a proxima tela de ranking deve mostrar, e para onde ela volta.
    struct PedidoDeRanking {
        bool geral = true;                                 ///< media do curso, e nao uma materia
        int  materia = 0;                                  ///< so vale quando geral e falso
        Scene::SceneType voltarPara = Scene::SceneType::MainMenu;
    };

    void PedirRanking(const PedidoDeRanking& pedido) { mPedidoDeRanking = pedido; }
    [[nodiscard]] const PedidoDeRanking& RankingPedido() const { return mPedidoDeRanking; }

    /// A matricula de uma posicao, ou Matricula::kVisitante.
    [[nodiscard]] const std::string& MatriculaDe(size_t posicao) const;

    /// A aparencia de uma posicao. Vazia quer dizer "a padrao".
    [[nodiscard]] const Personagens::Aparencia& AparenciaDe(size_t posicao) const;

    /// Troca a aparencia e grava, se o aluno estiver identificado.
    void DefinirAparencia(const Personagens::Aparencia& aparencia);

    /// Se o progresso desta posicao vai para o disco. Visitante nao grava.
    [[nodiscard]] bool ProgressoEGravado(size_t posicao) const;

    // Verifica se uma matéria específica está desbloqueada para jogar.
    bool IsStageUnlocked(int subject);

private:
    /// Passam as opcoes de som e de janela para o AudioSystem e o SDL.
    void AplicarVolumes();
    void AplicarTelaCheia();

    /// O progresso que o mapa do curso le: o do jogador 1.
    [[nodiscard]] const Progresso& ProgressoDoMapa() const { return mSessoes.front().progresso; }

    /// A sessao da posicao que esta na tela de matricula ou de personagem.
    [[nodiscard]] Sessao::Dados& SessaoEntrando();

    void ProcessInput();
    void UpdateGame();
    void GenerateOutput();

    // SDL stuff.
    class SDL_Window* mWindow;
    SDL_Renderer* mRenderer;
    std::unique_ptr<AudioSystem> mAudio;

    // Textura "faltando": xadrez magenta/preto gerado em memoria, devolvido sempre que um arquivo
    // de imagem nao carrega.
    SDL_Texture* mPlaceholderTexture = nullptr;

    // Cache de texturas: caminho do arquivo -> textura ja carregada na GPU. O Game e o dono de
    // todas elas e as destroi em Shutdown().
    std::map<std::string, SDL_Texture*> mTextureCache;

    // Cache dos quadros de cada folha: caminho do JSON -> retangulos.
    std::map<std::string, std::vector<SDL_Rect>> mQuadrosCache;

    // Window properties.
    int mWindowWidth;
    int mWindowHeight;

    // Track elapsed time since game start.
    Uint32 mTicksCount;

    // Track if game is running.
    bool mIsGameRunning;

    // Active scenes.
    std::unique_ptr<Scene> mScene;

    // All Boss factories As fabricas de chefe, pelo nome com que materias.json as chama.
    std::map<std::string, std::unique_ptr<IBossFactory>> mBossFactory;

    /// Quem esta jogando, uma entrada por posicao do painel.
    std::vector<Sessao::Dados> mSessoes{1};

    /// Quantos jogam esta partida. Posto pelo menu e so por ele.
    Sessao::Modo mModoDeJogo = Sessao::Modo::Um;
    Dificuldade::Nivel mDificuldade = Dificuldade::Nivel::Normal;

    size_t mPosicaoEntrando = 0;
    bool mEditandoPersonagem = false;
    size_t mPosicaoAntesDaEdicao = 0;

    PedidoDeRanking mPedidoDeRanking{};

    //Selected Stage, used by Battle on ChangeScene()
    int mSelectedStage{};
    int mMateriaEmFoco = -1;

    int mOrientador = -1;

    /// As opcoes da maquina (Configuracao.h). Lidas do disco em
    /// LoadInitialScene, depois que a pasta base e descoberta.
    Configuracao::Dados mConfiguracao{};

    bool mPendingSceneChange;
    Scene::SceneType mNextScene;

    // ---- Gabinete ----

    Gabinete::Configuracao mGabinete;
    Experimento::Condicao mCondicao;

    // ---- A simulacao (Simulacao.h) ----
    Simulacao::Configuracao mSimulacao;
    int mQuadro = 0;
    /// O teclado de verdade, com as teclas do roteiro apertadas por cima.
    std::vector<Uint8> mTecladoDoRoteiro;
    /// Os codigos das teclas do roteiro, na ordem dele; lidos no primeiro quadro, com o SDL de pe.
    std::vector<SDL_Scancode> mCodigosDoRoteiro;
    /// O teclado deste quadro: o de verdade, ou ele com o roteiro por cima.
    const Uint8* TecladoDoQuadro(const Uint8* real, int quantasTeclas);
    /// Fotografa a tela, se este quadro esta na lista; depois da ultima, sai.
    void FotografarSePedido();

    /// Tempo sem ninguem mexer. Esgotando, volta ao menu.
    Gabinete::Contagem mOciosidade;

    /// Tempo com a combinacao de saida segurada. Esgotando, fecha o jogo.
    Gabinete::Contagem mSaidaDoOperador;

    /// Lidos em ProcessInput e consumidos em UpdateGame, que e onde existe
    /// deltaTime.
    bool mHouveEntrada = false;
    bool mSaidaSegurada = false;

    /// A volta ao menu e a saida do operador, quadro a quadro. A tela cheia
    /// nao esta aqui: ela acontece uma vez so, em Initialize.
    void AtualizarGabinete(float deltaTime);

    /// Monta a ficha de uma posicao e grava. O unico lugar que faz isso, para nenhum caminho de
    /// gravacao esquecer um campo.
    bool GravarFicha(size_t posicao);

    void InitializeBossFactory();

    void ChangeScene(Scene::SceneType sceneType);

    bool HasPassed(int subject);

    int CountPassedInList(const std::vector<int>& subjects);
};
