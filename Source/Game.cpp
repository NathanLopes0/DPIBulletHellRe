//
// Created by Nathan on 03/04/2024.
//
#include "Game.h"
#include <SDL_ttf.h>
#include "SDL_image.h"
#include "Random.h"
#include "AudioSystem.h"
#include "Font.h"
#include "Components/DrawComponents/DrawComponent.h"
#include "Scenes/MainMenu.h"
#include "Scenes/StageSelect.h"
#include "Scenes/Battle/Battle.h"
#include "Actors/Teachers/BossFactory/AllFactories.h"


Game::Game(int windowWidth, int windowHeight)
    :mWindow(nullptr),
    mRenderer(nullptr),
    mWindowWidth(windowWidth),
    mWindowHeight(windowHeight),
    mTicksCount(0),
    mIsGameRunning(true),
    mScene(nullptr),
    mSelectedStage(INF250),
    mPendingSceneChange(false),
    mNextScene(Scene::SceneType::None)
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
    SDL_SetRenderDrawBlendMode(mRenderer, SDL_BLENDMODE_BLEND);

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

    // Put all Grades to 40;
    InitializeGrades();
    LoadInitialScene();

    return true;
}

//função que seleciona a cena inicial e chama a função Load.
void Game::LoadInitialScene()
{

    InitializeBossFactory();
    ChangeScene(Scene::SceneType::Battle);

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

    if (mPendingSceneChange)
    {
        ChangeScene(mNextScene);
        mPendingSceneChange = false;
        mNextScene = Scene::SceneType::None;
    }

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
// Em Game.cpp

void Game::Shutdown()
{
    // --- 1. LIMPEZA DOS RECURSOS DO JOGO ---

    // Destrói manualmente a cena (e todos os Atores/Componentes/Texturas)
    // Isso garante que ~DrawTextComponent() seja chamado
    // enquanto o Renderer ainda está vivo.
    mScene.reset();

    // Limpe quaisquer outros sistemas criados e que
    // dependem do SDL (como áudio ou fontes gerenciadas)

    mAudio.reset();
    mBossFactory.clear();

    // --- 2. DESLIGAMENTO DOS SISTEMAS SDL ---

    // Agora que todas as texturas foram destruídas, é seguro
    // desligar o TTF, o Renderer e o resto do SDL.

    TTF_Quit();
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

    SDL_Log(" GAME -- Inicializando todas as fabricas");
    mBossFactory[INF213] = std::make_unique<SallesFactory>(this);
    mBossFactory[INF250] = std::make_unique<RicardoFactory>(this);

    if (mBossFactory[INF213] && mBossFactory[INF250])
        SDL_Log(" GAME -- Fabricas inicializadas");

}

IBossFactory *Game::GetFactory(size_t n) {
    if (const auto it = mBossFactory.find(static_cast<GameSubject>(n)); it != mBossFactory.end()) {
        return it->second.get();
    }
    return nullptr;
}

void Game::InitializeGrades() {

    mGrades[INF213] = 40;
    mGrades[INF250] = 40;

}

Scene::SceneType Game::GetCurrSceneType() const {
    return mScene->GetType();
}

void Game::RequestSceneChange(const Scene::SceneType nextScene) {
    mPendingSceneChange = true;
    mNextScene = nextScene;
}

// Função auxiliar simples: Passou se nota >= 60
bool Game::HasPassed(const GameSubject subject) {

    // GetGrade retorna 0.0 se a matéria ainda não foi jogada, então funciona.
    return GetGrade(subject) >= 60.0f;
}

// Função genérica para contar aprovações em uma lista
int Game::CountPassedInList(const std::vector<GameSubject>& subjects) {
    int passedCount = 0;
    for (const auto& s : subjects) {
        if (HasPassed(s)) {
            passedCount++;
        }
    }
    return passedCount;
}

bool Game::IsStageUnlocked(GameSubject subject) {
    // ---------------------------------------------------------
    // REGRA 1: INF 213 (Primeira Coluna) é sempre desbloqueada
    // ---------------------------------------------------------
    if (subject == GameSubject::INF213) return true;

    // Definindo as colunas (conforme seu StageSelect.cpp)
    const std::vector<GameSubject> col2 = {
        GameSubject::INF250, GameSubject::INF220,
        GameSubject::INF330, GameSubject::INF332
    };

    const std::vector<GameSubject> col3 = {
        GameSubject::INF420, GameSubject::BIOINF,
        GameSubject::INF394, GameSubject::VISCCP
    };

    // ---------------------------------------------------------
    // REGRA 2: Coluna 2 desbloqueia se passou em INF 213
    // ---------------------------------------------------------
    // Verifica se o 'subject' atual está na lista da coluna 2
    for (auto s : col2) {
        if (s == subject) {
            return HasPassed(GameSubject::INF213);
        }
    }

    // ---------------------------------------------------------
    // REGRA 3: Coluna 3 desbloqueia se passou em 2 matérias da Coluna 2
    // ---------------------------------------------------------
    for (auto s : col3) {
        if (s == subject) {
            return CountPassedInList(col2) >= 2;
        }
    }

    // ---------------------------------------------------------
    // REGRA 4: TCC desbloqueia se passou em 2 matérias da Coluna 3
    // ---------------------------------------------------------
    if (subject == GameSubject::TCC) {
        return CountPassedInList(col3) >= 2;
    }

    // Por segurança, bloqueia qualquer coisa desconhecida
    return false;
}

