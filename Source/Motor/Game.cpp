//
// Created by Nathan on 03/04/2024.
//
#include "Game.h"
#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include "Json.h"
#include <SDL_ttf.h>
#include "SDL_image.h"
#include "Random.h"
#include "../Regras/Destrutivel.h"
#include "AudioSystem.h"
#include "Font.h"
#include "../Components/DrawComponents/DrawComponent.h"
#include "../Scenes/MainMenu.h"
#include "../Aluno/FichaArquivo.h"
#include "../Aluno/Horas.h"
#include "../Curso/MateriasArquivo.h"
#include "../Personagem/PersonagensArquivo.h"
#include "../Aluno/ExportacaoArquivo.h"
#include "Relogio.h"
#include "../Aluno/Matricula.h"
#include "../Scenes/Identificacao.h"
#include "../Scenes/Opcoes.h"
#include "../Scenes/TelaDeRanking.h"
#include "../Scenes/TelaDoBoletim.h"
#include "../Scenes/TelaDasHoras.h"
#include "../Scenes/TelaComoJogar.h"
#include "../Scenes/Atracao.h"
#include "../Scenes/CriacaoDePersonagem.h"
#include "../Scenes/StageSelect.h"
#include "../Scenes/EscolhaDoOrientador.h"
#include "../Scenes/EscolhaDaDificuldade.h"
#include "../Scenes/Battle/Battle.h"
#include "../Actors/Teachers/BossFactory/Chefes.h"
#include "../Actors/Actor.h"

#include "CaminhosArquivo.h"
#include "ConfiguracaoArquivo.h"

Game::Game(int windowWidth, int windowHeight, const Gabinete::Configuracao& gabinete,
           const Simulacao::Configuracao& simulacao)
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
    mCondicao(Experimento::Ler(gabinete.condicao)),
    mSimulacao(simulacao),
    mOciosidade(gabinete.ociosidade),
    // A saida do operador existe so no gabinete.
    mSaidaDoOperador(gabinete.arcade ? Gabinete::kSegurarParaSair : 0.0f)
{

}

Game::~Game() = default;

bool Game::Initialize() {

    // A condicao do experimento vai para o log no comeco: quem aplica o estudo confere ali.
    {
        std::vector<std::string> problemas;
        Experimento::Ler(mGabinete.condicao, &problemas);
        for (const auto& p : problemas) SDL_Log("EXPERIMENTO: %s; ignorada.", p.c_str());
        SDL_Log("EXPERIMENTO: condicao %s.", mCondicao.Nome().c_str());
    }
    if (mSimulacao.Ativa()) SDL_Log("SIMULACAO: o jogo esta rodando com parametros de simulacao.");

    if(SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    // Fora dele nasce do tamanho pedido, que e como se desenvolve.
    const Uint32 bandeirasDaJanela = mGabinete.arcade ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0u;
    // A simulacao pode pedir outra janela (--janela): o jogo continua posicionado em 1200x800, e a
    // escala logica do renderizador reconcilia, como no gabinete.
    const int larguraDaJanela = mSimulacao.larguraDaJanela > 0 ? mSimulacao.larguraDaJanela : mWindowWidth;
    const int alturaDaJanela = mSimulacao.alturaDaJanela > 0 ? mSimulacao.alturaDaJanela : mWindowHeight;
    mWindow = SDL_CreateWindow("DPI Bullet Hell", 0, 0, larguraDaJanela, alturaDaJanela,
                               bandeirasDaJanela);
    if(!mWindow)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    // Posicionando no centro da tela.
    SDL_SetWindowPosition(mWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    // Criando render.
    mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!mRenderer)
    {
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        return false;
    }
    SDL_SetRenderDrawBlendMode(mRenderer, SDL_BLENDMODE_BLEND);

    // Com a escala, o SDL amplia e centraliza, e nenhuma coordenada do jogo muda.
    SDL_RenderSetLogicalSize(mRenderer, mWindowWidth, mWindowHeight);

    if (mGabinete.arcade) {
        // Nao ha mouse no painel.
        SDL_ShowCursor(SDL_DISABLE);
    }

    // Iniciando sistema de fontes.
    if (TTF_Init() != 0)
    {
        SDL_Log("Failed to initialize SDL_ttf");
        return false;
    }

    mAudio = std::make_unique<AudioSystem>();

    // O sorteio: novo a cada abertura, ou preso pela simulacao (--semente).
    if (mSimulacao.semente) Random::Seed(*mSimulacao.semente);
    else Random::Init();

    // Start counter for deltaTime.
    mTicksCount = SDL_GetTicks();

    LoadInitialScene();

    return true;
}

// Função que seleciona a cena inicial e chama a função Load.
void Game::LoadInitialScene()
{

    // A base precisa estar descoberta antes de qualquer Caminhos::Asset().
    if (!Caminhos::Inicializar()) {
        SDL_Log("ERRO: a pasta Assets nao foi encontrada. O jogo vai abrir sem "
                "sprite, sem fonte e sem som, e nao vai salvar progresso.");
    }

    // As opcoes depois da base: o arquivo mora em Saves, que e relativa a ela.
    mConfiguracao = ConfiguracaoArquivo::Carregar();
    AplicarVolumes();
    AplicarTelaCheia();

    // Os sons sao carregados aqui, de uma vez, e nao no primeiro PlaySound de cada um.
    mAudio->CacheAllSounds();

    InitializeBossFactory();

    // A simulacao pode entrar direto numa batalha (--batalha), pelo mesmo caminho do jogo: as sessoes
    // refeitas por DefinirModoDeJogo, que e o que a batalha pergunta para criar os alunos.
    if (!mSimulacao.batalha.empty()) {
        const std::string& b = mSimulacao.batalha;
        const bool numero = b.find_first_not_of("0123456789") == std::string::npos;
        mSelectedStage = numero ? std::atoi(b.c_str()) : Materias::Carregadas().IndiceDe(b);
        DefinirModoDeJogo(mSimulacao.jogadores >= 2 ? Sessao::Modo::Dois : Sessao::Modo::Um);
        if (!mSimulacao.orientador.empty()) {
            DefinirOrientador(Materias::Carregadas().IndiceDe(mSimulacao.orientador));
        }
        // O modo e sempre escrito: a batalha da simulacao nao passa pela tela de dificuldade.
        DefinirDificuldade(mSimulacao.dificil ? Dificuldade::Nivel::Dificil : Dificuldade::Nivel::Normal);
        // --candidatos: a tela do orientador (ou a de --cena), com quem passou nessas materias - a
        // primeira com 100, e cada uma 8 a menos que a anterior.
        if (!mSimulacao.candidatos.empty() && !mSessoes.empty()) {
            float nota = 100.0f;
            std::string resto = mSimulacao.candidatos;
            while (!resto.empty()) {
                const size_t virgula = resto.find(',');
                const int materia = Materias::Carregadas().IndiceDe(resto.substr(0, virgula));
                if (materia >= 0) mSessoes.front().progresso.RegistrarNota(materia, nota, Dificuldade::Nivel::Normal);
                nota -= 8.0f;
                resto = virgula == std::string::npos ? std::string() : resto.substr(virgula + 1);
            }
            // Com --orientador a batalha comeca direto, ja com as notas: e como se confere o resultado.
            if (mSimulacao.cena == "selecao") ChangeScene(Scene::SceneType::StageSelect);
            else if (mSimulacao.cena == "boletim") ChangeScene(Scene::SceneType::Boletim);
            else if (mSimulacao.cena == "horas") ChangeScene(Scene::SceneType::Horas);
            else if (mSimulacao.orientador.empty()) ChangeScene(Scene::SceneType::EscolhaDoOrientador);
            else ChangeScene(Scene::SceneType::Battle);
            return;
        }
        ChangeScene(Scene::SceneType::Battle);
        return;
    }
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
                // No gabinete, fechar a janela nao existe.
                if (!mGabinete.arcade) Quit();
                break;
            case SDL_WINDOWEVENT:
                // Quem sai da janela no meio da luta nao quer voltar e achar a nota no chao.
                if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST && mScene) mScene->AoPerderOFoco();
                break;
            default: ;
        }
    }

    int quantasTeclas = 0;
    // Em duas linhas: a ordem dos argumentos de uma chamada nao e garantida, e quantasTeclas so vale
    // depois do SDL_GetKeyboardState.
    const Uint8* real = SDL_GetKeyboardState(&quantasTeclas);
    const Uint8* state = TecladoDoQuadro(real, quantasTeclas);

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
    // Calculo de deltaTime pro resto do jogo.
    {
        // Com passo fixo (--passo), nao ha relogio a esperar: o quadro anda o quanto a maquina der.
        if (mSimulacao.passo <= 0.0f) while(!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16)) {}

        deltaTime = (static_cast<float>(SDL_GetTicks()) - static_cast<float>(mTicksCount)) / 1000.0f;
        if(deltaTime > 0.05f)
        {
            deltaTime = 0.05f;
        }

        mTicksCount = SDL_GetTicks();
        if (mSimulacao.passo > 0.0f) deltaTime = mSimulacao.passo;
    }

    if (mScene) {
        mScene->Update(deltaTime);
    }
    // Os canais de som que terminaram voltam a ficar livres.
    if (mAudio) mAudio->Update();
    AtualizarGabinete(deltaTime);

    if (mPendingSceneChange)
    {
        // O pedido e consumido antes da troca, e nao depois.
        const Scene::SceneType proxima = mNextScene;
        mPendingSceneChange = false;
        mNextScene = Scene::SceneType::None;

        ChangeScene(proxima);
    }

}

namespace {

    /// Para onde a ociosidade leva, conforme a tela em que ela acontece. None quer dizer "fica onde
    /// esta".
    Scene::SceneType ParaOndeAOciosidadeLeva(const Scene::SceneType atual) {
        switch (atual) {
            case Scene::SceneType::Battle:
            case Scene::SceneType::Atracao:  return Scene::SceneType::None;
            case Scene::SceneType::MainMenu: return Scene::SceneType::Atracao;
            default:                         return Scene::SceneType::MainMenu;
        }
    }
}

void Game::AtualizarGabinete(const float deltaTime)
{
    // A saida de quem mantem a maquina.
    if (mSaidaDoOperador.Passou(deltaTime, mSaidaSegurada)) {
        SDL_Log("GABINETE: Ctrl+Esc segurado por %.0fs, fechando o jogo.",
                static_cast<double>(Gabinete::kSegurarParaSair));
        Quit();
        return;
    }

    if (!mOciosidade.Ligada() || !mScene) return;

    const Scene::SceneType destino = ParaOndeAOciosidadeLeva(GetCurrSceneType());

    // Um pedido de troca ja feito pela cena nao pode ser atropelado por este.
    if (destino == Scene::SceneType::None || mPendingSceneChange) {
        mOciosidade.Reiniciar();
        return;
    }

    if (mOciosidade.Passou(deltaTime, !mHouveEntrada)) {
        // O perfil de quem jogou nao pode ficar pendurado.
        SDL_Log("GABINETE: %.0fs parado, indo para a cena %d.",
                static_cast<double>(mGabinete.ociosidade), static_cast<int>(destino));
        JogarComoVisitante();
        RequestSceneChange(destino);
    }
}

void Game::GenerateOutput()
{
    // Set draw color to black.
    SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);

    // Clear back buffer.
    SDL_RenderClear(mRenderer);

    if (mScene) {
        const auto drawables = mScene->GetDrawables();
        for (const auto drawable : drawables) {
            drawable->Draw(mRenderer);
        }
    }

    ++mQuadro;
    FotografarSePedido();

    // Swap front buffer and back buffer.
    SDL_RenderPresent(mRenderer);
}

const Uint8* Game::TecladoDoQuadro(const Uint8* real, const int quantasTeclas) {
    if (mSimulacao.teclas.empty() || !real || quantasTeclas <= 0) return real;
    if (mCodigosDoRoteiro.empty()) {
        for (const Simulacao::Tecla& t : mSimulacao.teclas) {
            const SDL_Scancode codigo = SDL_GetScancodeFromName(t.nome.c_str());
            if (codigo == SDL_SCANCODE_UNKNOWN) SDL_Log("SIMULACAO: tecla desconhecida no roteiro: %s", t.nome.c_str());
            mCodigosDoRoteiro.push_back(codigo);
        }
    }
    mTecladoDoRoteiro.assign(real, real + quantasTeclas);
    for (size_t i = 0; i < mSimulacao.teclas.size(); ++i) {
        const SDL_Scancode codigo = mCodigosDoRoteiro[i];
        if (codigo == SDL_SCANCODE_UNKNOWN || codigo >= quantasTeclas) continue;
        if (Simulacao::Apertada(mSimulacao.teclas[i], mQuadro)) mTecladoDoRoteiro[static_cast<size_t>(codigo)] = 1;
    }
    return mTecladoDoRoteiro.data();
}

void Game::FotografarSePedido() {
    const auto& fotos = mSimulacao.fotos;
    if (fotos.empty() || std::find(fotos.begin(), fotos.end(), mQuadro) == fotos.end()) return;

    // A saida inteira do renderizador, em pixels de verdade: sem a escala logica durante a leitura.
    int largura = 0, altura = 0;
    SDL_GetRendererOutputSize(mRenderer, &largura, &altura);
    if (SDL_Surface* foto = SDL_CreateRGBSurfaceWithFormat(0, largura, altura, 32, SDL_PIXELFORMAT_ARGB8888)) {
        SDL_RenderSetLogicalSize(mRenderer, 0, 0);
        SDL_RenderSetScale(mRenderer, 1.0f, 1.0f);
        SDL_RenderSetViewport(mRenderer, nullptr);
        SDL_RenderReadPixels(mRenderer, nullptr, SDL_PIXELFORMAT_ARGB8888, foto->pixels, foto->pitch);
        char arquivo[1024];
        std::snprintf(arquivo, sizeof(arquivo), "%s/foto_%05d.bmp", mSimulacao.pastaDasFotos.c_str(), mQuadro);
        if (SDL_SaveBMP(foto, arquivo) != 0) SDL_Log("SIMULACAO: nao consegui gravar %s.", arquivo);
        SDL_FreeSurface(foto);
        SDL_RenderSetLogicalSize(mRenderer, mWindowWidth, mWindowHeight);
    }
    // Depois da ultima foto, o trabalho acabou.
    if (mQuadro >= *std::max_element(fotos.begin(), fotos.end())) std::exit(0);
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

    // Quem chama confere com TexturaGuardada antes de montar outra.
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

    // O placeholder e compartilhado por todo caminho de textura ausente e tem dono proprio.
    if (it->second != nullptr && it->second != mPlaceholderTexture) {
        SDL_DestroyTexture(it->second);
    }
    mTextureCache.erase(it);
}

const std::vector<SDL_Rect>& Game::QuadrosDaFolha(const std::string& dataPath) {
    if (const auto it = mQuadrosCache.find(dataPath); it != mQuadrosCache.end()) return it->second;

    std::vector<SDL_Rect> quadros;
    try {
        std::ifstream arquivo(dataPath);
        if (!arquivo.is_open()) throw std::runtime_error("arquivo nao encontrado");
        const nlohmann::json folha = nlohmann::json::parse(arquivo);
        for (const auto& frame : folha["frames"]) {
            quadros.push_back({frame["frame"]["x"].get<int>(), frame["frame"]["y"].get<int>(),
                               frame["frame"]["w"].get<int>(), frame["frame"]["h"].get<int>()});
        }
    } catch (const std::exception& e) {
        // Uma vez por caminho, porque o vazio tambem fica guardado.
        SDL_Log("SPRITE SHEET INVALIDA: %s (%s) -- usando frame unico de fallback",
                dataPath.c_str(), e.what());
        quadros.clear();
    }
    return mQuadrosCache.emplace(dataPath, std::move(quadros)).first->second;
}

SDL_Texture* Game::LoadTexture(const std::string& texturePath) {

    if (const auto it = mTextureCache.find(texturePath); it != mTextureCache.end()) {
        return it->second;
    }

    auto loadedIMG = IMG_Load(texturePath.c_str());
    if (loadedIMG == nullptr) {
        SDL_Log("TEXTURA AUSENTE: %s (%s) -- usando placeholder xadrez",
                texturePath.c_str(), IMG_GetError());

        // Guarda o placeholder sob o caminho que falhou.
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

SDL_Texture* Game::LoadSilhueta(const std::string& texturePath) {
    const std::string chave = texturePath + "#silhueta";
    if (SDL_Texture* pronta = TexturaGuardada(chave)) return pronta;

    SDL_Surface* lida = IMG_Load(texturePath.c_str());
    if (!lida) return nullptr;
    // RGBA32 e R, G, B, A nessa ordem na memoria, em qualquer maquina.
    SDL_Surface* rgba = SDL_ConvertSurfaceFormat(lida, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(lida);
    if (!rgba) return nullptr;

    if (SDL_LockSurface(rgba) == 0) {
        for (int y = 0; y < rgba->h; ++y) {
            auto* linha = static_cast<Uint8*>(rgba->pixels) + y * rgba->pitch;
            for (int x = 0; x < rgba->w; ++x) {
                Uint8* p = linha + 4 * x;
                p[0] = p[1] = p[2] = 255;
                p[3] = Destrutivel::AlfaDaSilhueta(p[3]);
            }
        }
        SDL_UnlockSurface(rgba);
    }
    SDL_Texture* silhueta = SDL_CreateTextureFromSurface(mRenderer, rgba);
    SDL_FreeSurface(rgba);
    if (!silhueta) return nullptr;
    SDL_SetTextureBlendMode(silhueta, SDL_BLENDMODE_BLEND);
    GuardarTextura(chave, silhueta);
    return silhueta;
}

void Game::Shutdown()
{
    // ---- 1. limpeza dos recursos do jogo ----

    mScene.reset();

    // Precisa ser antes de SDL_DestroyRenderer.
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

    mAudio.reset();
    mBossFactory.clear();

    // ---- 2. desligamento dos sistemas SDL ----

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
        case Scene::SceneType::Boletim:
            mScene = std::make_unique<TelaDoBoletim>(this);
            break;
        case Scene::SceneType::Horas:
            mScene = std::make_unique<TelaDasHoras>(this);
            break;
        case Scene::SceneType::ComoJogar:
            mScene = std::make_unique<TelaComoJogar>(this);
            break;
        case Scene::SceneType::Atracao:
            mScene = std::make_unique<Atracao>(this);
            break;
        case Scene::SceneType::StageSelect:
            mScene = std::make_unique<StageSelect>(this);
            break;
        case Scene::SceneType::EscolhaDoOrientador:
            mScene = std::make_unique<EscolhaDoOrientador>(this);
            break;
        case Scene::SceneType::EscolhaDaDificuldade:
            mScene = std::make_unique<EscolhaDaDificuldade>(this);
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

    // As fabricas sao registradas por nome, e nao por materia.
    mBossFactory = Chefes::Fabricas(this);
}

IBossFactory *Game::GetFactory(const size_t n) {

    // O indice deixa de significar nada fora da lista de materias, que e o ponto.
    const Materias::Materia* materia = Materias::Carregadas().Por(static_cast<int>(n));
    if (materia == nullptr) return nullptr;

    if (materia->chefe.empty()) return nullptr;   // materia ainda sem chefe

    const auto it = mBossFactory.find(materia->chefe);
    if (it == mBossFactory.end()) {
        // Nome escrito errado no arquivo.
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

    // As regras vivem em materias.json, nao mais aqui. Na dupla, a materia abre so se estiver aberta para
    // os dois: quem esta mais atras no curso nao joga o que ainda nao liberou.
    std::vector<const Progresso*> progressos;
    for (const Sessao::Dados& s : mSessoes) progressos.push_back(&s.progresso);
    return Materias::Carregadas().DesbloqueadaParaTodos(subject, progressos);
}

std::vector<size_t> Game::FechadaPara(const int subject) const {
    std::vector<size_t> quem;
    for (size_t i = 0; i < mSessoes.size(); ++i) {
        if (!Materias::Carregadas().Desbloqueada(subject, mSessoes[i].progresso)) quem.push_back(i);
    }
    return quem;
}

bool Game::ComprarMateria(const int subject) {
    const Materias::Lista& materias = Materias::Carregadas();
    const std::vector<size_t> quem = FechadaPara(subject);
    if (quem.empty()) return false;
    for (const size_t i : quem) {
        if (Horas::PodeComprar(materias, subject, mSessoes[i].progresso) != Horas::Compra::Pode) return false;
    }
    const std::string chave = Materias::ChaveDaCompra(materias.CodigoDe(subject));
    for (const size_t i : quem) {
        mSessoes[i].progresso.RegistrarAtividade(chave);
        if (ProgressoEGravado(i) && !GravarFicha(i)) {
            SDL_Log("GAME: nao consegui gravar a compra de %s na ficha de %s; ela vale para esta sessao.",
                    chave.c_str(), mSessoes[i].matricula.c_str());
        }
    }
    SDL_Log("GAME: %s comprada com %d h por %zu aluno(s).", chave.c_str(), materias.PrecoDe(subject), quem.size());
    return true;
}

// Quem esta jogando.

Sessao::Dados& Game::SessaoEntrando() {
    // A posicao 0 existe desde o construtor.
    return mPosicaoEntrando < mSessoes.size() ? mSessoes[mPosicaoEntrando] : mSessoes.front();
}

const std::string& Game::MatriculaDe(const size_t posicao) const {
    return posicao < mSessoes.size() ? mSessoes[posicao].matricula : mSessoes.front().matricula;
}

const Personagens::Aparencia& Game::AparenciaDe(const size_t posicao) const {
    return posicao < mSessoes.size() ? mSessoes[posicao].aparencia : mSessoes.front().aparencia;
}

void Game::AplicarVolumes() {
    if (mAudio) mAudio->DefinirVolumes(Configuracao::FatorDoVolume(mConfiguracao.volumeMusica),
                                       Configuracao::FatorDoVolume(mConfiguracao.volumeEfeitos));
}

void Game::AplicarTelaCheia() {
    // No gabinete a janela e sempre cheia.
    if (!mWindow || mGabinete.arcade) return;
    SDL_SetWindowFullscreen(mWindow, mConfiguracao.telaCheia ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0u);
}

void Game::DefinirVolumeDaMusica(const int volume) {
    const int v = Configuracao::MoverVolume(volume, 0);
    if (mConfiguracao.volumeMusica == v) return;
    mConfiguracao.volumeMusica = v;
    AplicarVolumes();
    ConfiguracaoArquivo::Gravar(mConfiguracao);
}

void Game::DefinirVolumeDosEfeitos(const int volume) {
    const int v = Configuracao::MoverVolume(volume, 0);
    if (mConfiguracao.volumeEfeitos == v) return;
    mConfiguracao.volumeEfeitos = v;
    AplicarVolumes();
    ConfiguracaoArquivo::Gravar(mConfiguracao);
}

void Game::DefinirTelaCheia(const bool cheia) {
    if (mConfiguracao.telaCheia == cheia) return;
    mConfiguracao.telaCheia = cheia;
    AplicarTelaCheia();
    ConfiguracaoArquivo::Gravar(mConfiguracao);
}

void Game::DefinirHitbox(const bool mostrar) {
    if (mConfiguracao.mostrarHitbox == mostrar) return;
    mConfiguracao.mostrarHitbox = mostrar;
    ConfiguracaoArquivo::Gravar(mConfiguracao);
}

void Game::DefinirModoDeJogo(const Sessao::Modo modo) {

    mModoDeJogo = modo;
    mPosicaoEntrando = 0;
    mDificuldade = Dificuldade::Nivel::Normal;
    mMateriaEmFoco = -1;

    // As sessoes sao refeitas, e nao ajustadas.
    mSessoes.assign(Sessao::Quantos(modo), Sessao::Dados{});
}

void Game::AvancarEntrada() {

    if (Sessao::Proximo(mModoDeJogo, mPosicaoEntrando) == Sessao::Passo::OutroJogador) {
        ++mPosicaoEntrando;
        RequestSceneChange(Scene::SceneType::Identificacao);
        return;
    }

    // Depois de todos entrarem, a dificuldade - e so entao as fases.
    RequestSceneChange(Scene::SceneType::EscolhaDaDificuldade);
}

void Game::CancelarEntrada() {
    mEditandoPersonagem = false;
    DefinirModoDeJogo(Sessao::Modo::Um);
}

bool Game::Formado(const size_t posicao) const {
    if (posicao >= mSessoes.size()) return false;
    const Personagens::Formatura& f = Personagens::Carregado().formatura;
    if (!f.Existe()) return false;
    const int materia = Materias::Carregadas().IndiceDe(f.materia);
    return materia >= 0 && mSessoes[posicao].progresso.Aprovado(materia);
}

void Game::EditarPersonagem(const size_t posicao, const Scene::SceneType voltarPara) {
    if (posicao >= mSessoes.size()) return;
    mVoltarDaEdicao = voltarPara;
    mPosicaoAntesDaEdicao = mPosicaoEntrando;
    mPosicaoEntrando = posicao;
    mEditandoPersonagem = true;
    RequestSceneChange(Scene::SceneType::CriacaoDePersonagem);
}

void Game::TerminarEdicaoDePersonagem() {
    mPosicaoEntrando = mPosicaoAntesDaEdicao;
    mEditandoPersonagem = false;
    RequestSceneChange(mVoltarDaEdicao);
}

float Game::RecordeDaPartida(const int materia) const {
    const auto modo = QuantosJogadores() > 1 ? Progresso::Modo::Dupla : Progresso::Modo::Solo;
    return ProgressoDoMapa().Recorde(materia, modo, NivelDeDificuldade()).nota;
}

std::vector<std::string> Game::RegistrarAtividades(const size_t posicao, const std::vector<std::string>& chaves) {
    std::vector<std::string> novas;
    if (posicao >= mSessoes.size()) return novas;
    for (const std::string& chave : chaves) {
        if (mSessoes[posicao].progresso.RegistrarAtividade(chave)) novas.push_back(chave);
    }
    return novas;
}

void Game::RegistrarNota(const int subject, const float nota) {

    // A mesma nota na ficha de cada um.
    const std::string quando = Relogio::Agora();
    bool gravouAlguma = false;

    // Quem decide o registro e o modo, e nao a nota.
    const bool emDupla = QuantosJogadores() > 1;

    for (size_t quem = 0; quem < mSessoes.size(); ++quem) {

        if (emDupla) {
            mSessoes[quem].progresso.RegistrarNotaDeDupla(
                subject, nota, NivelDeDificuldade(), Sessao::ParceiroDe(mSessoes, quem), quando);
        }
        else {
            mSessoes[quem].progresso.RegistrarNota(subject, nota, NivelDeDificuldade(), quando);
        }

        // Aprovado na materia que pede orientador: o professor que orientou entra no curriculo.
        const Materias::Materia* m = Materias::Carregadas().Por(subject);
        if (m && m->orientacao && mOrientador >= 0 && nota >= Progresso::kNotaDeAprovacao) {
            mSessoes[quem].progresso.RegistrarOrientacao(mOrientador);
        }

        // A ficha vai para o disco aqui, e nao em quem chama.
        if (!ProgressoEGravado(quem)) continue;

        if (GravarFicha(quem)) {
            gravouAlguma = true;
        }
        else {
            // Nao derruba a batalha nem avisa em tela: o aluno acabou de jogar e o que ele quer e ver a nota.
            SDL_Log("GAME: nao consegui gravar a ficha da matricula %s. A nota desta "
                    "batalha vale para esta sessao, mas nao foi para o disco.",
                    mSessoes[quem].matricula.c_str());
        }
    }

    // A planilha do professor e regravada uma vez, e nao uma por aluno.
    if (gravouAlguma) Exportacao::Regravar(Materias::Carregadas());
}

bool Game::GravarFicha(const size_t posicao) {

    if (posicao >= mSessoes.size()) return false;

    // Um so lugar monta a ficha a partir do estado da sessao.
    const Sessao::Dados& sessao = mSessoes[posicao];

    Ficha::Dados dados;
    dados.matricula = sessao.matricula;
    dados.progresso = sessao.progresso;
    dados.aparencia = sessao.aparencia;

    return FichaArquivo::Gravar(dados, Materias::Carregadas());
}

void Game::IdentificarAluno(const std::string& matriculaCanonica) {

    Sessao::Dados& sessao = SessaoEntrando();
    sessao.matricula = matriculaCanonica;

    // Substitui progresso E aparencia.
    const Ficha::Dados ficha = FichaArquivo::Carregar(sessao.matricula, Materias::Carregadas());
    sessao.progresso = ficha.progresso;
    sessao.aparencia = ficha.aparencia;

    SDL_Log("GAME: jogador %zu e a matricula %s (%zu materia(s) com registro, aparencia %s).",
            mPosicaoEntrando + 1, sessao.matricula.c_str(), sessao.progresso.QuantasRegistradas(),
            sessao.aparencia.Vazia() ? "padrao" : "do save");
}

void Game::DefinirAparencia(const Personagens::Aparencia& aparencia) {

    SessaoEntrando().aparencia = aparencia;

    if (!ProgressoEGravado(mPosicaoEntrando)) return;   // visitante nao grava

    if (!GravarFicha(mPosicaoEntrando)) {
        SDL_Log("GAME: nao consegui gravar a aparencia da matricula %s. Ela vale para esta "
                "sessao, mas nao foi para o disco.", SessaoEntrando().matricula.c_str());
    }
}

void Game::JogarComoVisitante() {

    Sessao::Dados& sessao = SessaoEntrando();
    sessao.matricula = Matricula::kVisitante;
    sessao.progresso = Progresso();
    sessao.aparencia = Personagens::Aparencia();

    SDL_Log("GAME: jogador %zu sem identificacao. O progresso NAO sera gravado.",
            mPosicaoEntrando + 1);
}

bool Game::ProgressoEGravado(const size_t posicao) const {
    // Visitante nao grava.
    return Matricula::Validar(MatriculaDe(posicao)).valida;
}
