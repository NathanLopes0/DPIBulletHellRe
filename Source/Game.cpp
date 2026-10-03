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
#include "FichaArquivo.h"
#include "MateriasArquivo.h"
#include "ExportacaoArquivo.h"
#include "Relogio.h"
#include "Matricula.h"
#include "Scenes/Identificacao.h"
#include "Scenes/Opcoes.h"
#include "Scenes/StageSelect.h"
#include "Scenes/Battle/Battle.h"
#include "Actors/Teachers/BossFactory/AllFactories.h"

#include "CaminhosArquivo.h"


Game::Game(int windowWidth, int windowHeight)
    :mWindow(nullptr),
    mRenderer(nullptr),
    mWindowWidth(windowWidth),
    mWindowHeight(windowHeight),
    mTicksCount(0),
    mIsGameRunning(true),
    mScene(nullptr),
    mSelectedStage(INF213),
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
    LoadInitialScene();

    return true;
}

//função que seleciona a cena inicial e chama a função Load.
void Game::LoadInitialScene()
{

    // A base precisa estar descoberta ANTES de qualquer Caminhos::Asset().
    // As fabricas de projetil montam os caminhos delas na lista de
    // inicializacao do construtor, que roda dentro de InitializeBossFactory.
    if (!Caminhos::Inicializar()) {
        SDL_Log("ERRO: a pasta Assets nao foi encontrada. O jogo vai abrir sem "
                "sprite, sem fonte e sem som, e nao vai salvar progresso.");
    }

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
    float deltaTime = 0;
    // Calculo de deltaTime pro resto do jogo
    {
        while(!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16)) {}

        deltaTime = (static_cast<float>(SDL_GetTicks()) - static_cast<float>(mTicksCount)) / 1000.0f;
        if(deltaTime > 0.05f)
        {
            deltaTime = 0.05f;
        }

        mTicksCount = SDL_GetTicks();
    }

    if (mScene) {
        mScene->Update(deltaTime);
    }
    UpdateCamera();

    if (mPendingSceneChange)
    {
        // O PEDIDO E CONSUMIDO ANTES DA TROCA, e nao depois. ChangeScene chama o
        // Load() da cena nova, e um Load() pode pedir outra troca - e o que a
        // Battle faz quando a materia nao tem BossFactory. Limpando depois, esse
        // pedido era apagado no instante em que era feito, e a fase quebrada
        // ficava na tela para sempre, com o log dizendo que estava voltando.
        const Scene::SceneType proxima = mNextScene;
        mPendingSceneChange = false;
        mNextScene = Scene::SceneType::None;

        ChangeScene(proxima);
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
SDL_Texture* Game::GetPlaceholderTexture() {

    // Criada uma unica vez e reaproveitada por todos os caminhos que falharem.
    if (mPlaceholderTexture) {
        return mPlaceholderTexture;
    }

    constexpr int kSize = 64;   // lado da textura, em pixels
    constexpr int kCell = 8;    // lado de cada quadrado do xadrez

    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, kSize, kSize, 32,
                                                          SDL_PIXELFORMAT_RGBA32);
    if (!surface) {
        SDL_Log("Nao foi possivel criar a superficie do placeholder: %s", SDL_GetError());
        return nullptr;
    }

    // Magenta puro sobre quase-preto: o xadrez classico de textura ausente.
    // A cor e escolhida justamente por nao aparecer em arte de verdade, entao
    // e impossivel confundir com um sprite que ficou estranho.
    const Uint32 magenta = SDL_MapRGBA(surface->format, 255, 0, 220, 255);
    const Uint32 escuro  = SDL_MapRGBA(surface->format, 24, 0, 24, 255);

    SDL_FillRect(surface, nullptr, magenta);

    for (int linha = 0; linha < kSize / kCell; ++linha) {
        for (int coluna = 0; coluna < kSize / kCell; ++coluna) {
            if ((linha + coluna) % 2 == 0) {
                SDL_Rect celula{ coluna * kCell, linha * kCell, kCell, kCell };
                SDL_FillRect(surface, &celula, escuro);
            }
        }
    }

    mPlaceholderTexture = SDL_CreateTextureFromSurface(mRenderer, surface);
    SDL_FreeSurface(surface);

    if (!mPlaceholderTexture) {
        SDL_Log("Nao foi possivel criar a textura do placeholder: %s", SDL_GetError());
    }

    return mPlaceholderTexture;
}

SDL_Texture* Game::LoadTexture(const std::string& texturePath) {

    // Se essa imagem ja foi carregada, devolve a MESMA textura.
    if (const auto it = mTextureCache.find(texturePath); it != mTextureCache.end()) {
        return it->second;
    }

    auto loadedIMG = IMG_Load(texturePath.c_str());
    if (loadedIMG == nullptr) {
        SDL_Log("TEXTURA AUSENTE: %s (%s) -- usando placeholder xadrez",
                texturePath.c_str(), IMG_GetError());

        // Guarda o placeholder SOB O CAMINHO QUE FALHOU. Sem isso, cada
        // componente criado tentaria o IMG_Load de novo: com 300 projeteis de
        // pre-aquecimento seriam 300 leituras de disco falhas e 300 linhas de
        // log identicas.
        SDL_Texture* placeholder = GetPlaceholderTexture();
        if (placeholder) {
            mTextureCache.emplace(texturePath, placeholder);
        }
        return placeholder;
    }

    auto textureFromSur = SDL_CreateTextureFromSurface(mRenderer, loadedIMG);
    SDL_FreeSurface(loadedIMG);
    if (!textureFromSur)
    {
        SDL_Log("FALHA AO CRIAR TEXTURA de %s: %s -- usando placeholder",
                texturePath.c_str(), SDL_GetError());
        SDL_Texture* placeholder = GetPlaceholderTexture();
        if (placeholder) {
            mTextureCache.emplace(texturePath, placeholder);
        }
        return placeholder;
    }

    mTextureCache.emplace(texturePath, textureFromSur);

    return textureFromSur;
}

void Game::Shutdown()
{
    // --- 1. LIMPEZA DOS RECURSOS DO JOGO ---

    // Destrói manualmente a cena (e todos os Atores/Componentes/Texturas)
    // Isso garante que ~DrawTextComponent() seja chamado
    // enquanto o Renderer ainda está vivo.
    mScene.reset();

    // Com a cena morta, nenhum Component referencia mais as texturas do cache,
    // entao e seguro destrui-las. Precisa ser ANTES de SDL_DestroyRenderer.
    // O placeholder aparece no cache sob VARIOS caminhos (um para cada arquivo
    // que faltou), mas e o mesmo ponteiro. Destrui-lo dentro do laco seria
    // double free. Por isso ele e pulado aqui e destruido uma unica vez depois.
    for (auto& [path, texture] : mTextureCache) {
        if (texture && texture != mPlaceholderTexture) {
            SDL_DestroyTexture(texture);
        }
    }
    mTextureCache.clear();

    if (mPlaceholderTexture) {
        SDL_DestroyTexture(mPlaceholderTexture);
        mPlaceholderTexture = nullptr;
    }

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
        case Scene::SceneType::Identificacao:
            mScene = std::make_unique<Identificacao>(this);
            break;
        case Scene::SceneType::Opcoes:
            mScene = std::make_unique<Opcoes>(this);
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

    mBossFactory[INF213] = std::make_unique<SallesFactory>(this);
    mBossFactory[INF250] = std::make_unique<RicardoFactory>(this);
    mBossFactory[INF330] = std::make_unique<AndreFactory>(this);
    mBossFactory[INF420] = std::make_unique<JulioFactory>(this);


}

IBossFactory *Game::GetFactory(size_t n) {
    if (const auto it = mBossFactory.find(static_cast<GameSubject>(n)); it != mBossFactory.end()) {
        return it->second.get();
    }
    return nullptr;
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

    // Le o RECORDE, nao a ultima nota: assim uma tentativa ruim nao re-tranca
    // uma materia que o jogador ja tinha passado.
    return mProgresso.Aprovado(static_cast<int>(subject));
}

// Função genérica para contar aprovações em uma lista
int Game::CountPassedInList(const std::vector<GameSubject>& subjects) {
    std::vector<int> materias;
    materias.reserve(subjects.size());
    for (const auto& s : subjects) materias.push_back(static_cast<int>(s));
    return mProgresso.QuantasAprovadas(materias);
}

bool Game::IsStageUnlocked(GameSubject subject) {
    // ---------------------------------------------------------
    // REGRA 1: INF 213 (Primeira Coluna) é sempre desbloqueada
    // ---------------------------------------------------------
    if (subject == GameSubject::INF213) return true;

    // Definindo as colunas (conforme o StageSelect está definido)
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


// ---------------------------------------------------------------------------
// Quem esta jogando
// ---------------------------------------------------------------------------

void Game::RegistrarNota(const GameSubject subject, const float nota) {

    mProgresso.RegistrarNota(static_cast<int>(subject), nota, Relogio::Agora());

    // A ficha vai para o disco AQUI, e nao em quem chama: assim nenhum caminho de
    // fim de batalha pode esquecer. Visitante nao grava - e o combinado com quem
    // escolheu nao se identificar.
    if (!ProgressoEGravado()) return;

    if (FichaArquivo::Gravar(mMatricula, mProgresso, Materias::Carregadas())) {
        // A planilha do professor e regravada junto, entao ela esta sempre em dia e
        // ninguem precisa lembrar de exportar. Ela e DERIVADA das fichas: apagar
        // notas.csv nao perde nada, volta na proxima batalha.
        Exportacao::Regravar(Materias::Carregadas());
    }
    else {
        // Nao derruba a batalha nem avisa em tela: o aluno acabou de jogar e o que
        // ele quer e ver a nota. O log diz o que houve para quem for investigar.
        SDL_Log("GAME: nao consegui gravar a ficha da matricula %s. A nota desta "
                "batalha vale para esta sessao, mas nao foi para o disco.",
                mMatricula.c_str());
    }
}

void Game::IdentificarAluno(const std::string& matriculaCanonica) {

    mMatricula = matriculaCanonica;

    // SUBSTITUI o progresso inteiro. Carregar por cima do anterior deixaria as
    // notas do aluno que acabou de sair penduradas na sessao de quem entrou - e
    // elas iriam para o disco na primeira batalha, no arquivo errado.
    mProgresso = FichaArquivo::Carregar(mMatricula, Materias::Carregadas());

    SDL_Log("GAME: jogando como a matricula %s (%zu materia(s) com registro).",
            mMatricula.c_str(), mProgresso.QuantasRegistradas());
}

void Game::JogarComoVisitante() {
    mMatricula = Matricula::kVisitante;
    mProgresso = Progresso();
    SDL_Log("GAME: jogando sem identificacao. O progresso NAO sera gravado.");
}

bool Game::ProgressoEGravado() const {
    // Visitante nao grava. O teste e por Matricula::Validar e nao por comparacao
    // com o rotulo: assim qualquer valor que nao seja uma matricula de verdade -
    // inclusive a string vazia do jogo recem-aberto - cai do lado certo.
    return Matricula::Validar(mMatricula).valida;
}
