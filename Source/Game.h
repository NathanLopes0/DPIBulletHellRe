//
// Created by nslop on 03/04/2024.
//

#pragma once

#include <map>
#include <string>
#include <memory>
#include <SDL.h>
#include "Math.h"
#include "Scenes/Scene.h"
#include "Personagens.h"
#include "Progresso.h"
#include "Gabinete.h"

class IBossFactory;
class Actor;

class Game {
public:

    // UMA MATERIA E O INDICE DELA em Materias::Carregadas().
    //
    // Aqui havia um enum com as dez materias escritas a mao. Ele nao descrevia
    // mais nada que o arquivo nao dissesse - chefe, coluna e desbloqueio ja vem
    // de la - e so dava a impressao de que a lista morava no C++. O indice NAO
    // vai para o disco: o save grava o codigo da materia.

    /// @param gabinete o que a linha de comando pediu. O padrao e janela; ver
    ///        Gabinete.h para o que muda no arcade.
    Game(int windowWidth, int windowHeight, const Gabinete::Configuracao& gabinete = {});
    ~Game();

    bool Initialize();
    void RunLoop();
    void Shutdown();
    void Quit() { mIsGameRunning = false; }

    //Actor functions
    void LoadInitialScene();

    //Texture
    class SDL_Texture* LoadTexture(const std::string& texturePath);

    /**
     * @brief A textura guardada sob esta chave, ou nullptr.
     *
     * Para texturas que NAO vem de arquivo - a personagem composta e a primeira
     * delas. A chave nao e um caminho; ver Personagens::ChaveDaAparencia.
     */
    [[nodiscard]] class SDL_Texture* TexturaGuardada(const std::string& chave) const;

    /**
     * @brief Guarda uma textura ja pronta. O Game PASSA A SER DONO dela.
     *
     * Entra no mesmo cache das texturas de arquivo de proposito: assim ela e
     * destruida no mesmo lugar que todas as outras, sem regra de posse nova. Uma
     * chave ja ocupada e recusada, para nao vazar a textura que estava la.
     */
    void GuardarTextura(const std::string& chave, class SDL_Texture* textura);

    /**
     * @brief Destroi e esquece uma textura guardada.
     *
     * Existe para a tela de criacao de personagem, que compoe uma textura nova
     * a cada seta apertada. Sem isto, trocar de cabelo cem vezes deixaria cem
     * texturas vivas ate o jogo fechar - e numa maquina compartilhada, que fica
     * ligada o dia todo, isso se acumula aluno apos aluno.
     *
     * QUEM CHAMA GARANTE que ninguem mais aponta para ela.
     */
    void EsquecerTextura(const std::string& chave);

    /**
     * @brief Devolve (criando na primeira chamada) o xadrez magenta/preto usado
     * como marcador de textura ausente.
     *
     * E gerada em memoria de proposito: um placeholder que precisasse ser lido
     * do disco poderia falhar exatamente pelo mesmo motivo que a textura
     * original falhou.
     */
    SDL_Texture* GetPlaceholderTexture();

    //Camera functions
    Vector2& GetCameraPos() { return mCameraPos; }
    void SetCameraPos(const Vector2& position) { mCameraPos = position; };
    class SDL_Renderer* GetRenderer() { return mRenderer; }

    // Window functions
    [[nodiscard]] int GetWindowWidth() const { return mWindowWidth; }
    [[nodiscard]] int GetWindowHeight() const { return mWindowHeight; }

    //Audio System
    class AudioSystem *GetAudio() { return mAudio.get(); }

    //Scenes
    [[nodiscard]] Scene* GetScene() const { return mScene.get(); }
    void RequestSceneChange(Scene::SceneType nextScene);


    [[nodiscard]] Scene::SceneType GetCurrSceneType() const;

    IBossFactory* GetFactory(size_t n);

    // Funções pra retornar o estágio escolhido em StageSelect (retorna INF213 caso não tenha sido modificado,
    // porque é oq foi definido no construtor)
    [[nodiscard]] int GetSelectedStage() const { return mSelectedStage; }
    void SetSelectedStage(const int subject) { mSelectedStage = subject; }

    // ---- Notas -------------------------------------------------------------
    // O mapa unico de notas foi trocado por Progresso, que separa duas coisas
    // que antes eram a mesma: o RECORDE de cada materia e o PONTO DE RETOMADA
    // da proxima batalha. A nota da batalha em curso nao fica mais aqui - ela
    // vive em Battle::mGrade, que e onde ela muda.

    /// @brief A maior nota que o jogador ja tirou. Zero se nunca jogou.
    /// E o que a tela de selecao mostra e o que as regras de desbloqueio leem.
    /// A sobrecarga que existia aqui para receber um int cru sumiu junto com o
    /// enum: agora ela seria a mesma funcao.
    [[nodiscard]] float GetMelhorNota(const int materia) const {
        return mProgresso.MelhorNota(materia);
    }

    /// @brief A ultima nota que o jogador tirou nesta materia. NAO e por onde a
    /// batalha comeca - toda fase parte de Progresso::kNotaInicial.
    [[nodiscard]] float GetUltimaNota(const int materia) const {
        return mProgresso.UltimaNota(materia);
    }

    /// @brief Registra o resultado de uma batalha: a retomada passa a ser esta
    /// nota, e o recorde so sobe. Chamado UMA vez, ao fim da batalha.
    ///
    /// E TAMBEM onde a ficha do aluno vai para o disco. Gravar aqui, e nao em quem
    /// chama, e o que garante que nenhum caminho de fim de batalha esqueca - era
    /// assim que a nota chegava ao armazenamento por efeito colateral antes de o
    /// Progresso existir, e nao quero repetir o padrao.
    void RegistrarNota(int subject, float nota);

    /**
     * @brief Passa a jogar como este aluno, carregando a ficha dele do disco.
     *
     * A matricula ja deve vir CANONICA (ver Matricula::Validar). Trocar de aluno
     * substitui o progresso inteiro: as notas do anterior nao podem ficar
     * penduradas na sessao do seguinte.
     */
    void IdentificarAluno(const std::string& matriculaCanonica);

    /**
     * @brief Passa a jogar sem se identificar.
     *
     * O progresso da sessao comeca zerado e NAO e gravado. Quem escolhe isto ve o
     * aviso na tela antes, nao depois.
     */
    void JogarComoVisitante();

    /// @brief A matricula de quem esta jogando, ou Matricula::kVisitante.
    /// Por qual caminho do menu o aluno chegou a tela de matricula.
    enum class ModoDeEntrada {
        Novo,       ///< vai CRIAR um perfil; matricula que ja tem perfil e recusada
        Carregar    ///< vai ABRIR um perfil; matricula sem perfil e recusada
    };

    /// @brief Diz a que veio a proxima tela de matricula.
    ///
    /// Fica no Game, e nao num parametro da cena, pelo mesmo motivo de
    /// mSelectedStage: quem cria as cenas e o ChangeScene, que nao conhece a
    /// intencao de quem pediu a troca.
    void DefinirModoDeEntrada(const ModoDeEntrada modo) { mModoDeEntrada = modo; }
    [[nodiscard]] ModoDeEntrada ModoDeEntradaAtual() const { return mModoDeEntrada; }

    /**
     * @brief O que a proxima tela de ranking deve mostrar, e para onde ela volta.
     *
     * Fica no Game pelo mesmo motivo de mSelectedStage e de mModoDeEntrada: quem
     * cria as cenas e o ChangeScene, que nao conhece a intencao de quem pediu a
     * troca. Sem isto, a tela nao teria como saber se foi aberta pelo menu (media
     * do curso, volta para o menu) ou pela selecao de fases (uma materia so,
     * volta para a selecao).
     */
    struct PedidoDeRanking {
        bool geral = true;                                 ///< media do curso, e nao uma materia
        int  materia = 0;                                  ///< so vale quando geral e falso
        Scene::SceneType voltarPara = Scene::SceneType::MainMenu;
    };

    void PedirRanking(const PedidoDeRanking& pedido) { mPedidoDeRanking = pedido; }
    [[nodiscard]] const PedidoDeRanking& RankingPedido() const { return mPedidoDeRanking; }

    [[nodiscard]] const std::string& MatriculaAtual() const { return mMatricula; }

    /// @brief A aparencia de quem esta jogando. Vazia quer dizer "a padrao".
    [[nodiscard]] const Personagens::Aparencia& AparenciaAtual() const { return mAparencia; }

    /**
     * @brief Troca a aparencia e grava, se o aluno estiver identificado.
     *
     * Grava na hora, e nao no fim da batalha: quem acaba de montar a personagem
     * espera encontra-la do mesmo jeito ao carregar o perfil, mesmo que feche o
     * jogo sem jogar nada.
     */
    void DefinirAparencia(const Personagens::Aparencia& aparencia);

    /// @brief Se o progresso desta sessao vai para o disco.
    [[nodiscard]] bool ProgressoEGravado() const;

    // Verifica se uma matéria específica está desbloqueada para jogar
    bool IsStageUnlocked(int subject);

private:
    void ProcessInput();
    void UpdateGame();
    void UpdateCamera();
    void GenerateOutput();

    // SDL stuff
    class SDL_Window* mWindow;
    SDL_Renderer* mRenderer;
    std::unique_ptr<AudioSystem> mAudio;

    // Textura "faltando": xadrez magenta/preto gerado em memoria, devolvido
    // sempre que um arquivo de imagem nao carrega. E compartilhada por todos os
    // caminhos que falharam, entao NAO pode ser destruida junto com o cache.
    SDL_Texture* mPlaceholderTexture = nullptr;

    // Cache de texturas: caminho do arquivo -> textura ja carregada na GPU.
    // O Game e o DONO de todas elas e as destroi em Shutdown(). Nenhum
    // DrawComponent deve chamar SDL_DestroyTexture na textura que recebeu.
    std::map<std::string, SDL_Texture*> mTextureCache;

    // Window properties
    int mWindowWidth;
    int mWindowHeight;

    // Track elapsed time since game start
    Uint32 mTicksCount;

    // Track if game is running
    bool mIsGameRunning;

    // Camera X and Y position
    Vector2 mCameraPos;

    //Active scenes
    std::unique_ptr<Scene> mScene;

    //All Boss factories
    /// As fabricas de chefe, pelo NOME com que materias.json as chama.
    std::map<std::string, std::unique_ptr<IBossFactory>> mBossFactory;

    /// Recordes e pontos de retomada. Ver Progresso.h.
    Progresso mProgresso;

    /// Quem esta jogando. Comeca como visitante, entao abrir o jogo e sair sem
    /// passar pela identificacao nunca grava nada por engano.
    std::string mMatricula;

    /// A que veio a proxima tela de matricula. Ver DefinirModoDeEntrada.
    ModoDeEntrada mModoDeEntrada = ModoDeEntrada::Novo;
    PedidoDeRanking mPedidoDeRanking{};

    /// A aparencia de quem esta jogando, COMO VEIO DO SAVE. Vazia quer dizer
    /// "use a padrao" - e o caso do visitante e de todo save anterior a versao 4.
    /// Quem compoe conserta o que nao existir mais no catalogo.
    Personagens::Aparencia mAparencia;

    //Selected Stage, used by Battle on ChangeScene()
    int mSelectedStage{};

    bool mPendingSceneChange;
    Scene::SceneType mNextScene;

    // ---- Gabinete ----------------------------------------------------------
    // Ver Gabinete.h. As duas esperas sao a MESMA classe com a condicao
    // trocada, e o limite zero as desliga: por isso nao ha "if (arcade)" dentro
    // de AtualizarGabinete - fora do gabinete elas nascem desligadas.
    //
    // DECLARADAS DEPOIS de mNextScene porque e nessa ordem que o construtor as
    // inicializa, e o compilador avisa quando as duas listas divergem.

    Gabinete::Configuracao mGabinete;

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


    /// Monta a ficha a partir do estado da sessao e grava. O unico lugar que faz
    /// isso, para nenhum caminho de gravacao esquecer um campo.
    bool GravarFicha();

    void InitializeBossFactory();

    void ChangeScene(Scene::SceneType sceneType);

    bool HasPassed(int subject);

    int CountPassedInList(const std::vector<int>& subjects);
};
