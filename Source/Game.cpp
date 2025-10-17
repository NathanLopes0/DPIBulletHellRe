//
// Created by Nathan on 03/04/2024.
//
#include "Game.h"
#include <SDL_ttf.h>
#include "SDL_image.h"
#include "Random.h"
#include "AudioSystem.h"
#include "Font.h"
#include "Actors/Actor.h"
#include "Components/DrawComponents/DrawComponent.h"
#include "Scenes/MainMenu.h"
#include "Scenes/StageSelect.h"
#include "Scenes/Battle/Battle.h"
#include "Actors/Teacher/BossFactory/IBossFactory.h"
#include "Actors/Teacher/BossFactory/SallesFactory.h"


Game::Game(int windowWidth, int windowHeight)
    :mWindow(nullptr),
    mRenderer(nullptr),
    mWindowWidth(windowWidth),
    mWindowHeight(windowHeight),
    mTicksCount(0),
    mIsGameRunning(true),
    mScene(nullptr)
{

}

Game::~Game() = default;


bool Game::Initialize() {

    if(SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    //criando janela do jogo
    mWindow = SDL_CreateWindow("DPI Bullet Hell", 0, 0, mWindowWidth, mWindowHeight, 0);
    if(!mWindow)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    //posicionando no centro da tela
    SDL_SetWindowPosition(mWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    //criando render
    mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!mRenderer)
    {
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        return false;
    }

    //iniciando sistema de fontes
    if (TTF_Init() != 0)
    {
        SDL_Log("Failed to initialize SDL_ttf");
        return false;
    }

    mAudio = std::make_unique<AudioSystem>();

    //new random seed
    Random::Init();

    //start counter for deltaTime
    mTicksCount = SDL_GetTicks();

    // Init all game actors
    LoadInitialScene();

    // Put all Grades to 40;
    InitializeGrades();

    return true;
}

//função que seleciona a cena inicial e chama a função Load.
void Game::LoadInitialScene()
{

    InitializeBossFactory();
    ChangeScene(Scene::SceneType::MainMenu);

}

void Game::RunLoop() {

    while (mIsGameRunning)
    {
        ProcessInput();
        UpdateGame();
        GenerateOutput();
    }
}

void Game::ProcessInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                Quit();
                break;
            default: ;
        }
    }

    const Uint8* state = SDL_GetKeyboardState(nullptr);

    if (mScene) {
        mScene->ProcessInput(state);
    }
}

void Game::UpdateGame()
{
    while(!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16));

    float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
    if(deltaTime > 0.05f)
    {
        deltaTime = 0.05f;
    }

    mTicksCount = SDL_GetTicks();

    if (mScene) {
        mScene->Update(deltaTime);
    }
    UpdateCamera();

}

void Game::UpdateCamera()
{

}
void Game::GenerateOutput()
{
    // Set draw color to black
    SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);

    // Clear back buffer
    SDL_RenderClear(mRenderer);

    if (mScene) {
        const auto drawables = mScene->GetDrawables();
        for (const auto drawable : drawables) {
            drawable->Draw(mRenderer);
        }
    }

    // Swap front buffer and back buffer
    SDL_RenderPresent(mRenderer);
}
SDL_Texture* Game::LoadTexture(const std::string& texturePath) {

    auto loadedIMG = IMG_Load(texturePath.c_str());
    if (loadedIMG == nullptr) {
        SDL_Log("textura invalida queride");
        return nullptr;
    }


    auto textureFromSur = SDL_CreateTextureFromSurface(mRenderer, loadedIMG);
    SDL_FreeSurface(loadedIMG);
    if (!textureFromSur)
    {
        return nullptr;
    }


    return textureFromSur;
}
void Game::Shutdown()
{
    SDL_DestroyRenderer(mRenderer);
    SDL_DestroyWindow(mWindow);
    SDL_Quit();
}
void Game::ChangeScene(const Scene::SceneType sceneType)
{
    mAudio->StopAllSounds();

    switch (sceneType) {
        case Scene::SceneType::MainMenu:
            mScene = std::make_unique<MainMenu>(this);
            break;
        case Scene::SceneType::StageSelect:
            mScene = std::make_unique<StageSelect>(this);
            break;
        case Scene::SceneType::Battle:

            mScene = std::make_unique<Battle>(this, mSelectedStage);
            break;
        default:
            mScene = std::make_unique<MainMenu>(this);
            break;
    }

    mScene->Load();

}

void Game::InitializeBossFactory() {

    mBossFactory[GameSubject::INF213] = std::make_unique<SallesFactory>();
    //mBossFactory[GameSubject::INF250] = std::make_unique...

    //todas as outras, depois que criar as classes

}

IBossFactory *Game::GetFactory(size_t n) {
    if (const auto it = mBossFactory.find(static_cast<GameSubject>(n)); it != mBossFactory.end()) {
        return it->second.get();
    }
    return nullptr;
}

void Game::InitializeGrades() {

    mGrades[INF213] = 40;
    //mGrades[INF250] = 40;
    //todas as outras

}

Scene::SceneType Game::GetCurrSceneType() const {
    return mScene->GetType();
}
