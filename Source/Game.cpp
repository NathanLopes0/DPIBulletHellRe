//
// Created by nslop on 03/04/2024.
//
#include "Game.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <SDL_ttf.h>
#include "SDL_image.h"
#include "Random.h"
#include "AudioSystem.h"
#include "Actors/Actor.h"
#include "Components/DrawComponents/DrawComponent.h"
#include "Scenes/MainMenu.h"
#include "Scenes/StageSelect.h"
#include "Scenes/Battle.h"


Game::Game(int windowWidth, int windowHeight)
    :mWindow(nullptr),
    mRenderer(nullptr),
    mTicksCount(0),
    mIsRunning(true),
    mUpdatingActors(false),
    mWindowWidth(windowWidth),
    mWindowHeight(windowHeight)
{

}

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

    mAudio = new AudioSystem();

    //new random seed
    Random::Init();

    //start counter for deltaTime
    mTicksCount = SDL_GetTicks();

    // Init all game actors
    InitializeActors();

    return true;
}

//função que seleciona a cena inicial e chama a função Load.
void Game::InitializeActors()
{

    auto mainMenu = new MainMenu(this);
    mScene.push(mainMenu);
    mScene.top()->Load();

}

void Game::RunLoop() {

    while (mIsRunning)
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
        }
    }

    const Uint8* state = SDL_GetKeyboardState(nullptr);

    for (auto actor : mActors)
    {
        actor->ProcessInput(state);
    }

    if (!mScene.empty())
    {
        mScene.top()->ProcessInput(state);
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

    UpdateScenes(deltaTime);
    UpdateActors(deltaTime);
    UpdateCamera();

}

void Game::UpdateCamera()
{

}

void Game::UpdateActors(float deltaTime) {

    mUpdatingActors = true;
    for (auto actor : mActors)
    {
        actor->Update(deltaTime);
    }
    mUpdatingActors = false;

    for (auto pending : mPendingActors)
    {
        mActors.emplace_back(pending);
    }
    mPendingActors.clear();

    std::vector<Actor*> deadActors;
    for (auto actor : mActors)
    {
        if (actor->GetState() == ActorState::Destroy)
        {
            deadActors.emplace_back(actor);
        }
    }

    for (auto actor : deadActors)
    {
        delete actor;
    }
}

void Game::UpdateScenes(float deltaTime) {
    mScene.top()->Update(deltaTime);
}

void Game::AddActor(Actor* actor)
{
    if (mUpdatingActors)
    {
        mPendingActors.emplace_back(actor);
    }
    else
    {
        mActors.emplace_back(actor);
    }
}

void Game::RemoveActor(Actor* actor)
{
    auto iter = std::find(mPendingActors.begin(), mPendingActors.end(), actor);
    if (iter != mPendingActors.end())
    {
        // Swap to end of vector and pop off (avoid erase copies)
        std::iter_swap(iter, mPendingActors.end() - 1);
        mPendingActors.pop_back();
    }

    iter = std::find(mActors.begin(), mActors.end(), actor);
    if (iter != mActors.end())
    {
        // Swap to end of vector and pop off (avoid erase copies)
        std::iter_swap(iter, mActors.end() - 1);
        mActors.pop_back();
    }
}

void Game::AddDrawable(class DrawComponent *drawable)
{
    mDrawables.emplace_back(drawable);

    std::sort(mDrawables.begin(), mDrawables.end(),[](DrawComponent* a, DrawComponent* b) {
        return a->GetDrawOrder() < b->GetDrawOrder();
    });
}

void Game::RemoveDrawable(class DrawComponent *drawable)
{
    auto iter = std::find(mDrawables.begin(), mDrawables.end(), drawable);
    mDrawables.erase(iter);
}

void Game::AddCollider(CircleColliderComponent* collider)
{
    mColliders.emplace_back(collider);
}

void Game::RemoveCollider(CircleColliderComponent* collider)
{
    auto iter = std::find(mColliders.begin(), mColliders.end(), collider);
    mColliders.erase(iter);
}

void Game::GenerateOutput()
{
    // Set draw color to black
    SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);

    // Clear back buffer
    SDL_RenderClear(mRenderer);

    for (auto drawable : mDrawables)
    {
        if (drawable->IsVisible())
        {
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

    UnloadActors();
    UnloadScenes();

    SDL_DestroyRenderer(mRenderer);
    SDL_DestroyWindow(mWindow);
    SDL_Quit();
}

void Game::UnloadActors()
{
    while (!mActors.empty())
    {
        delete mActors.back();
    }

}

void Game::UnloadScenes() {
    while(!mScene.empty())
        mScene.pop();
}

void Game::SetScene(Scene::SceneType sceneType, bool RemoveLast)
{
    mAudio->StopAllSounds();

    Scene* scene;

    switch (sceneType) {
        case Scene::SceneType::MainMenu: {
            scene = new MainMenu(this);
            break;
        }
        case Scene::SceneType::StageSelect: {
            scene = new StageSelect(this);
            break;
        }
        case Scene::SceneType::Battle: {
            scene = new Battle(this);
            break;
        }
    }

    if(RemoveLast) {
        UnloadActors();
        UnloadScenes();
    }

    mScene.push(scene);
}

StageSelect* Game::GetStageSelect() {
    if(mScene.top()->GetType() == Scene::SceneType::StageSelect)
        return dynamic_cast<StageSelect *>(mScene.top());
    else
        return nullptr;
}
