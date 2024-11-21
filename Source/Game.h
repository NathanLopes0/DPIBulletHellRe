//
// Created by nslop on 03/04/2024.
//

#pragma once

#include <vector>
#include <stack>
#include <map>
#include <string>
#include <SDL.h>
#include "Math.h"
#include "Scenes/Scene.h"

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

    bool Initialize();
    void RunLoop();
    void Shutdown();
    void Quit() { mIsRunning = false; }

    //Actor functions
    void InitializeActors();
    void UpdateActors(float deltaTime);
    void AddActor(class Actor* actor);
    void RemoveActor(class Actor* actor);
    void UnloadActors();

    //Draw functions
    void AddDrawable(class DrawComponent* drawable);
    void RemoveDrawable(class DrawComponent* drawable);

    //Texture
    class SDL_Texture* LoadTexture(const std::string& texturePath);

    //Collider functions
    void AddCollider(class CircleColliderComponent* collider);
    void RemoveCollider(class CircleColliderComponent* collider);
    std::vector<class CircleColliderComponent*>& GetColliders() { return mColliders; }

    //Camera functions
    Vector2& GetCameraPos() { return mCameraPos; }
    void SetCameraPos(const Vector2& position) { mCameraPos = position; };
    class SDL_Renderer* GetRenderer() { return mRenderer; }

    // Window functions
    [[nodiscard]] int GetWindowWidth() const { return mWindowWidth; }
    [[nodiscard]] int GetWindowHeight() const { return mWindowHeight; }

    //Audio System
    class AudioSystem* GetAudio() { return mAudio; }

    //Scenes
    Scene* GetScene() { return mScene.top(); }
    void SetScene(Scene::SceneType sceneType, bool RemoveLast = true);

    Scene::SceneType GetCurrSceneType();

    //Used by Battle to get the selected stage
    class StageSelect* GetStageSelect();
    class BossFactory* GetFactory(int n);

    //Grade functions
    float GetGrade(int n) { return mGrades[static_cast<GameSubject>(n)]; }
    float GetGrade(GameSubject subject) { return mGrades[subject]; }
    void SetGrade(GameSubject subject, float grade) { mGrades[subject] = grade; }
    void SetGrade(int n, float grade) { mGrades[static_cast<GameSubject>(n)] = grade; }

private:
    void ProcessInput();
    void UpdateGame();
    void UpdateScenes(float deltaTime);
    void UpdateCamera();
    void GenerateOutput();
    void UnloadScenes();


    // All the actors in the game
    std::vector<class Actor*> mActors;
    std::vector<class Actor*> mPendingActors;

    // All the draw components
    std::vector<class DrawComponent*> mDrawables;

    // All the collision components
    std::vector<class CircleColliderComponent*> mColliders;

    // SDL stuff
    class SDL_Window* mWindow;
    SDL_Renderer* mRenderer;
    AudioSystem* mAudio;

    // Window properties
    int mWindowWidth;
    int mWindowHeight;

    // Track elapsed time since game start
    Uint32 mTicksCount;

    // Track if we're updating actors right now
    bool mIsRunning;
    bool mUpdatingActors;

    // Camera X and Y position
    Vector2 mCameraPos;

    //Stack of active scenes (normally only one, except on pause)
    std::stack<Scene*> mScene;

    //All Boss factories
    std::map<GameSubject, class BossFactory*> mBossFactory;

    //All Grades
    std::map<GameSubject, float> mGrades;

    void InitializeBossFactory();
    void InitializeGrades();
};
