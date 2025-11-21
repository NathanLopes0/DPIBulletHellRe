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

    //Camera functions
    Vector2& GetCameraPos() { return mCameraPos; }
    void SetCameraPos(const Vector2& position) { mCameraPos = position; };
    class SDL_Renderer* GetRenderer() { return mRenderer; }

    // Window functions
    [[nodiscard]] int GetWindowWidth() const { return mWindowWidth; }
    [[nodiscard]] int GetWindowHeight() const { return mWindowHeight; }

    //Audio System
    class AudioSystem* GetAudio() { return mAudio.get(); }

    //Scenes
    [[nodiscard]] Scene* GetScene() const { return mScene.get(); }
    void ChangeScene(Scene::SceneType sceneType);

    [[nodiscard]] Scene::SceneType GetCurrSceneType() const;

    class IBossFactory* GetFactory(size_t n);


    // Funções pra retornar o estágio escolhido em StageSelect (retorna INF213 caso não tenha sido modificado,
    // porque é oq foi definido no construtor)
    [[nodiscard]] GameSubject GetSelectedStage() const { return mSelectedStage; }
    void SetSelectedStage(const GameSubject subject) { mSelectedStage = subject; }

    //Grade functions
    float GetGrade(int n) { return mGrades[static_cast<GameSubject>(n)]; }
    float GetGrade(const GameSubject subject) { return mGrades[subject]; }
    void SetGrade(const GameSubject subject, const float grade) { mGrades[subject] = grade; }
    void SetGrade(int n, const float grade) { mGrades[static_cast<GameSubject>(n)] = grade; }

private:
    void ProcessInput();
    void UpdateGame();
    void UpdateCamera();
    void GenerateOutput();

    // SDL stuff
    class SDL_Window* mWindow;
    SDL_Renderer* mRenderer;
    std::unique_ptr<AudioSystem> mAudio;

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

    //All Grades
    std::map<GameSubject, float> mGrades;

    //Selected Stage, used by Battle on ChangeScene()
    GameSubject mSelectedStage{};


    void InitializeBossFactory();
    void InitializeGrades();
};
