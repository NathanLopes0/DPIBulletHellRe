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
#include "Progresso.h"

class IBossFactory;
class Actor;

class Game {
public:

    enum GameSubject {
        INF213,
        INF250,
        INF220,
        INF330,
        INF332,
        INF420,
        BIOINF,
        INF394,
        VISCCP,
        TCC
    };

    Game(int windowWidth, int windowHeight);
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
    [[nodiscard]] GameSubject GetSelectedStage() const { return mSelectedStage; }
    void SetSelectedStage(const GameSubject subject) { mSelectedStage = subject; }

    // ---- Notas -------------------------------------------------------------
    // O mapa unico de notas foi trocado por Progresso, que separa duas coisas
    // que antes eram a mesma: o RECORDE de cada materia e o PONTO DE RETOMADA
    // da proxima batalha. A nota da batalha em curso nao fica mais aqui - ela
    // vive em Battle::mGrade, que e onde ela muda.

    /// @brief A maior nota que o jogador ja tirou. Zero se nunca jogou.
    /// E o que a tela de selecao mostra e o que as regras de desbloqueio leem.
    [[nodiscard]] float GetMelhorNota(const GameSubject subject) const {
        return mProgresso.MelhorNota(static_cast<int>(subject));
    }
    [[nodiscard]] float GetMelhorNota(const int n) const { return mProgresso.MelhorNota(n); }

    /// @brief A ultima nota que o jogador tirou nesta materia. NAO e por onde a
    /// batalha comeca - toda fase parte de Progresso::kNotaInicial.
    [[nodiscard]] float GetUltimaNota(const GameSubject subject) const {
        return mProgresso.UltimaNota(static_cast<int>(subject));
    }

    /// @brief Registra o resultado de uma batalha: a retomada passa a ser esta
    /// nota, e o recorde so sobe. Chamado UMA vez, ao fim da batalha.
    ///
    /// E TAMBEM onde a ficha do aluno vai para o disco. Gravar aqui, e nao em quem
    /// chama, e o que garante que nenhum caminho de fim de batalha esqueca - era
    /// assim que a nota chegava ao armazenamento por efeito colateral antes de o
    /// Progresso existir, e nao quero repetir o padrao.
    void RegistrarNota(GameSubject subject, float nota);

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
    [[nodiscard]] const std::string& MatriculaAtual() const { return mMatricula; }

    /// @brief Se o progresso desta sessao vai para o disco.
    [[nodiscard]] bool ProgressoEGravado() const;

    // Verifica se uma matéria específica está desbloqueada para jogar
    bool IsStageUnlocked(GameSubject subject);

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
    std::map<GameSubject, std::unique_ptr<IBossFactory>> mBossFactory;

    /// Recordes e pontos de retomada. Ver Progresso.h.
    Progresso mProgresso;

    /// Quem esta jogando. Comeca como visitante, entao abrir o jogo e sair sem
    /// passar pela identificacao nunca grava nada por engano.
    std::string mMatricula;

    //Selected Stage, used by Battle on ChangeScene()
    GameSubject mSelectedStage{};

    bool mPendingSceneChange;
    Scene::SceneType mNextScene;


    void InitializeBossFactory();

    void ChangeScene(Scene::SceneType sceneType);

    bool HasPassed(GameSubject subject);

    int CountPassedInList(const std::vector<GameSubject>& subjects);
};
