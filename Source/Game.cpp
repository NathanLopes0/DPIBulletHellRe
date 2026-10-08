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
#include "Scenes/TelaDeRanking.h"
#include "Scenes/CriacaoDePersonagem.h"
#include "Scenes/StageSelect.h"
#include "Scenes/Battle/Battle.h"
#include "Actors/Teachers/BossFactory/AllFactories.h"

#include "CaminhosArquivo.h"


Game::Game(int windowWidth, int windowHeight, const Gabinete::Configuracao& gabinete)
    :mWindow(nullptr),
    mRenderer(nullptr),
    mWindowWidth(windowWidth),
    mWindowHeight(windowHeight),
    mTicksCount(0),
    mIsGameRunning(true),
    mScene(nullptr),
    mSelectedStage(0),
    mPendingSceneChange(false),
    mNextScene(Scene::SceneType::None),
    mGabinete(gabinete),
    mOciosidade(gabinete.ociosidade),
    // A saida do operador existe SO no gabinete. Fora dele o limite e zero, e
    // uma contagem de limite zero nunca esgota - a regra fica no dado em vez de
    // num "if" espalhado por quem chama.
    mSaidaDoOperador(gabinete.arcade ? Gabinete::kSegurarParaSair : 0.0f)
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
    // No gabinete a janela nasce em tela cheia, do tamanho do monitor que
    // estiver la. Fora dele nasce do tamanho pedido, que e como se desenvolve.
    const Uint32 bandeirasDaJanela = mGabinete.arcade ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0u;
    mWindow = SDL_CreateWindow("DPI Bullet Hell", 0, 0, mWindowWidth, mWindowHeight,
                               bandeirasDaJanela);
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

    // O JOGO INTEIRO E POSICIONADO EM mWindowWidth x mWindowHeight, inclusive
    // as telas que usam fracoes da altura. A tela do gabinete nao tem esse
    // tamanho, e sem escala logica tudo seria desenhado num canto dela. Com a
    // escala, o SDL amplia e centraliza, e NENHUMA coordenada do jogo muda.
    //
    // Fica fora do "if (arcade)" de proposito: numa janela do tamanho exato ela
    // nao faz nada, e assim existe um caminho so para os dois modos.
    SDL_RenderSetLogicalSize(mRenderer, mWindowWidth, mWindowHeight);

    if (mGabinete.arcade) {
        // Nao ha mouse no painel. Sem isto o cursor fica parado no meio da tela
        // o dia inteiro.
        SDL_ShowCursor(SDL_DISABLE);
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
                // No gabinete, fechar a janela NAO existe: o jogo fica aberto o
                // dia todo, e um Alt+F4 de aluno curioso deixaria a area de
                // trabalho a vista. A saida de quem mantem a maquina esta em
                // AtualizarGabinete.
                if (!mGabinete.arcade) Quit();
                break;
            default: ;
        }
    }

    int quantasTeclas = 0;
    const Uint8* state = SDL_GetKeyboardState(&quantasTeclas);

    // Guardados aqui e usados em AtualizarGabinete, que roda no UpdateGame
    // porque e la que existe deltaTime.
    mHouveEntrada = false;
    for (int i = 0; i < quantasTeclas; ++i) {
        if (state[i]) {
            mHouveEntrada = true;
            break;
        }
    }
    mSaidaSegurada = state[SDL_SCANCODE_LCTRL] && state[SDL_SCANCODE_ESCAPE];

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
    AtualizarGabinete(deltaTime);

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

void Game::AtualizarGabinete(const float deltaTime)
{
    // A SAIDA DE QUEM MANTEM A MAQUINA. Ctrl+Esc segurado por alguns segundos,
    // porque no painel nao existe nenhuma das duas teclas: para sair e preciso
    // ligar um teclado de proposito. Um toque nao derruba o jogo.
    if (mSaidaDoOperador.Passou(deltaTime, mSaidaSegurada)) {
        SDL_Log("GABINETE: Ctrl+Esc segurado por %.0fs, fechando o jogo.",
                static_cast<double>(Gabinete::kSegurarParaSair));
        Quit();
        return;
    }

    if (!mOciosidade.Ligada() || !mScene) return;

    // A BATALHA NAO VOLTA SOZINHA: ficar parado e uma forma legitima de
    // desviar, e ela ja termina por tempo. O menu tambem nao, porque e o
    // destino - e e dali que a tela de atracao vai sair, quando existir.
    const Scene::SceneType atual = GetCurrSceneType();
    const bool voltaSozinha = atual != Scene::SceneType::MainMenu
                           && atual != Scene::SceneType::Battle;

    // Um pedido de troca ja feito pela cena nao pode ser atropelado por este.
    if (!voltaSozinha || mPendingSceneChange) {
        mOciosidade.Reiniciar();
        return;
    }

    if (mOciosidade.Passou(deltaTime, !mHouveEntrada)) {
        // O PERFIL DE QUEM JOGOU NAO PODE FICAR PENDURADO. Quem chegasse depois
        // encontraria a sessao de outra pessoa e jogaria no nome dela.
        SDL_Log("GABINETE: %.0fs parado, voltando ao menu.",
                static_cast<double>(mGabinete.ociosidade));
        JogarComoVisitante();
        RequestSceneChange(Scene::SceneType::MainMenu);
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

SDL_Texture* Game::TexturaGuardada(const std::string& chave) const {
    const auto it = mTextureCache.find(chave);
    return it == mTextureCache.end() ? nullptr : it->second;
}

void Game::GuardarTextura(const std::string& chave, SDL_Texture* textura) {

    if (textura == nullptr) return;

    // Recusa em vez de sobrescrever: sobrescrevendo, a textura que estava na
    // chave ficaria sem dono e vazaria ate o jogo fechar. Quem chama confere
    // com TexturaGuardada antes de montar outra.
    if (mTextureCache.find(chave) != mTextureCache.end()) {
        SDL_Log("TEXTURA: a chave \"%s\" ja estava ocupada; a nova foi descartada.",
                chave.c_str());
        SDL_DestroyTexture(textura);
        return;
    }

    mTextureCache.emplace(chave, textura);
}

void Game::EsquecerTextura(const std::string& chave) {

    const auto it = mTextureCache.find(chave);
    if (it == mTextureCache.end()) return;

    // O placeholder e compartilhado por todo caminho de textura ausente e tem
    // dono proprio; destrui-lo aqui deixaria os outros com ponteiro morto.
    if (it->second != nullptr && it->second != mPlaceholderTexture) {
        SDL_DestroyTexture(it->second);
    }
    mTextureCache.erase(it);
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
        case Scene::SceneType::CriacaoDePersonagem:
            mScene = std::make_unique<CriacaoDePersonagem>(this);
            break;
        case Scene::SceneType::Opcoes:
            mScene = std::make_unique<Opcoes>(this);
            break;
        case Scene::SceneType::Ranking:
            mScene = std::make_unique<TelaDeRanking>(this);
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

    // AS FABRICAS SAO REGISTRADAS POR NOME, e nao por materia. Quem diz qual
    // chefe cada materia tem e o campo "chefe" de materias.json - ver
    // GetFactory. Antes este mapa era indexado pela materia, e havia entao duas
    // listas dizendo a mesma coisa: esta e a do arquivo. Mudar a ordem das
    // materias no arquivo bastava para a fase abrir com o chefe errado, sem erro
    // de compilacao e sem aviso em jogo.
    mBossFactory["salles"]  = std::make_unique<SallesFactory>(this);
    mBossFactory["ricardo"] = std::make_unique<RicardoFactory>(this);
    mBossFactory["andre"]   = std::make_unique<AndreFactory>(this);
    mBossFactory["julio"]   = std::make_unique<JulioFactory>(this);
    mBossFactory["thiago"]  = std::make_unique<ThiagoFactory>(this);
}

IBossFactory *Game::GetFactory(const size_t n) {

    // O caminho agora e materia -> nome do chefe (dados) -> fabrica (codigo),
    // em vez de indice -> fabrica. O indice deixa de significar nada fora da
    // lista de materias, que e o ponto: reordenar materias.json passa a ser
    // seguro.
    const Materias::Materia* materia = Materias::Carregadas().Por(static_cast<int>(n));
    if (materia == nullptr) return nullptr;

    if (materia->chefe.empty()) return nullptr;   // materia ainda sem chefe

    const auto it = mBossFactory.find(materia->chefe);
    if (it == mBossFactory.end()) {
        // Nome escrito errado no arquivo. Vale avisar: a materia existe, o
        // aluno consegue entrar e a fase volta sozinha para a selecao sem dizer
        // por que - ver Battle::LoadBoss.
        SDL_Log("GAME: a materia %s pede o chefe \"%s\", que nao existe. Chefes "
                "registrados: salles, ricardo, andre, julio.",
                materia->codigo.c_str(), materia->chefe.c_str());
        return nullptr;
    }

    return it->second.get();
}


Scene::SceneType Game::GetCurrSceneType() const {
    return mScene->GetType();
}

void Game::RequestSceneChange(const Scene::SceneType nextScene) {
    mPendingSceneChange = true;
    mNextScene = nextScene;
}

bool Game::IsStageUnlocked(const int subject) {

    // AS REGRAS VIVEM EM materias.json, nao mais aqui.
    //
    // O que havia nesta funcao era a TERCEIRA lista de materias escrita em C++:
    // quais estao em cada coluna, quem abre com o que, quantas aprovacoes cada
    // porta pede - tudo ja declarado no arquivo, no campo "desbloqueio", e tudo
    // ignorado ate agora. Materias::Lista::Desbloqueada implementa as mesmas
    // regras sobre os dados, e e pura e testada.
    //
    // A TROCA E SEGURA POR MEDIDA, e nao por leitura: ha um teste que compara as
    // duas respostas em 5120 combinacoes (512 estados de progresso x 10
    // materias) e exige que concordem. Ele foi escrito para este dia.
    return Materias::Carregadas().Desbloqueada(static_cast<int>(subject), mProgresso);
}


// ---------------------------------------------------------------------------
// Quem esta jogando
// ---------------------------------------------------------------------------

void Game::RegistrarNota(const int subject, const float nota) {

    mProgresso.RegistrarNota(static_cast<int>(subject), nota, Relogio::Agora());

    // A ficha vai para o disco AQUI, e nao em quem chama: assim nenhum caminho de
    // fim de batalha pode esquecer. Visitante nao grava - e o combinado com quem
    // escolheu nao se identificar.
    if (!ProgressoEGravado()) return;

    if (GravarFicha()) {
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

bool Game::GravarFicha() {

    // UM SO LUGAR monta a ficha a partir do estado da sessao. Com a aparencia
    // entrando no arquivo, gravar passou a ter dois campos para lembrar; dois
    // pontos de gravacao viravam duas chances de esquecer um deles.
    Ficha::Dados dados;
    dados.matricula = mMatricula;
    dados.progresso = mProgresso;
    dados.aparencia = mAparencia;

    return FichaArquivo::Gravar(dados, Materias::Carregadas());
}

void Game::IdentificarAluno(const std::string& matriculaCanonica) {

    mMatricula = matriculaCanonica;

    // SUBSTITUI progresso E aparencia. Carregar por cima do anterior deixaria as
    // notas do aluno que acabou de sair penduradas na sessao de quem entrou - e
    // elas iriam para o disco na primeira batalha, no arquivo errado. Com a
    // aparencia o estrago seria menos grave e igualmente confuso: o aluno novo
    // entraria com a cara do anterior.
    const Ficha::Dados ficha = FichaArquivo::Carregar(mMatricula, Materias::Carregadas());
    mProgresso = ficha.progresso;
    mAparencia = ficha.aparencia;

    SDL_Log("GAME: jogando como a matricula %s (%zu materia(s) com registro, aparencia %s).",
            mMatricula.c_str(), mProgresso.QuantasRegistradas(),
            mAparencia.Vazia() ? "padrao" : "do save");
}

void Game::DefinirAparencia(const Personagens::Aparencia& aparencia) {

    mAparencia = aparencia;

    if (!ProgressoEGravado()) return;   // visitante nao grava

    if (!GravarFicha()) {
        SDL_Log("GAME: nao consegui gravar a aparencia da matricula %s. Ela vale para esta "
                "sessao, mas nao foi para o disco.", mMatricula.c_str());
    }
}

void Game::JogarComoVisitante() {
    mMatricula = Matricula::kVisitante;
    mProgresso = Progresso();
    mAparencia = Personagens::Aparencia();
    SDL_Log("GAME: jogando sem identificacao. O progresso NAO sera gravado.");
}

bool Game::ProgressoEGravado() const {
    // Visitante nao grava. O teste e por Matricula::Validar e nao por comparacao
    // com o rotulo: assim qualquer valor que nao seja uma matricula de verdade -
    // inclusive a string vazia do jogo recem-aberto - cai do lado certo.
    return Matricula::Validar(mMatricula).valida;
}
