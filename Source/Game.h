//
// Created by nslop on 03/04/2024.
//

#ifndef DPIBULLETHELLRE_GAME_H
#define DPIBULLETHELLRE_GAME_H


class Game {
public:
    enum class GameScene
    {
        MainMenu,
        StageSelect,
        Battle
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

    //Draw functions
    void AddDrawable(class DrawComponent* drawable);
    void RemoveDrawable(class DrawComponent* drawable);

    //Texture
    SDL_Texture* LoadTexture(const std::string& texturePath);

    //Collider functions
    void AddCollider(class CircleColliderComponent* collider);
    void RemoveCollider(class CircleColliderComponent* collider);
    std::vector<class CircleColliderComponent*>& GetColliders() { return mColliders; }

    //Camera functions
    Vector2& GetCameraPos() { return mCameraPos; }
    void SetCameraPos(const Vector2& position) { mCameraPos = position; };
    SDL_Renderer* GetRenderer() { return mRenderer; }

    // Window functions
    int GetWindowWidth() const { return mWindowWidth; }
    int GetWindowHeight() const { return mWindowHeight; }

    //Audio System
    class AudioSystem* GetAudio() { return mAudio; }

    //Scenes
    void SetScene(GameScene scene);
    void UnloadActors();

private:
    void ProcessInput();
    void UpdateGame();
    void UpdateCamera();
    void GenerateOutput();


    // All the actors in the game
    std::vector<class Actor*> mActors;
    std::vector<class Actor*> mPendingActors;

    // All the draw components
    std::vector<class DrawComponent*> mDrawables;

    // All the collision components
    std::vector<class CircleColliderComponent*> mColliders;

    // SDL stuff
    SDL_Window* mWindow;
    SDL_Renderer* mRenderer;
    AudioSystem* mAudio;

    // Window properties
    int mWindowWidth;
    int mWindowHeight;
    int mGameWindowWidth;
    int mGameWindowHeight;

    // Track elapsed time since game start
    Uint32 mTicksCount;

    // Track if we're updating actors right now
    bool mIsRunning;
    bool mUpdatingActors;
    bool mChangeScene;

    Vector2 mCameraPos;
    GameScene mGameState;
    class Scene* mScene;

};


#endif //DPIBULLETHELLRE_GAME_H
