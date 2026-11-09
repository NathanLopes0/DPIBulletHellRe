//
// Created by Nathan on 03/04/2024.
//
#include "Game.h"
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
#include "../Curso/MateriasArquivo.h"
#include "../Personagem/PersonagensArquivo.h"
#include "../Aluno/ExportacaoArquivo.h"
#include "Relogio.h"
#include "../Aluno/Matricula.h"
#include "../Scenes/Identificacao.h"
#include "../Scenes/Opcoes.h"
#include "../Scenes/TelaDeRanking.h"
#include "../Scenes/TelaDoBoletim.h"
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
    // A saida do operador existe so no gabinete.
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

    // Fora dele nasce do tamanho pedido, que e como se desenvolve.
    const Uint32 bandeirasDaJanela = mGabinete.arcade ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0u;
    mWindow = SDL_CreateWindow("DPI Bullet Hell", 0, 0, mWindowWidth, mWindowHeight,
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

    // New random seed.
    Random::Init();

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
    const Uint8* state = SDL_GetKeyboardState(&quantasTeclas);

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

    // Swap front buffer and back buffer.
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

    // As regras vivem em materias.json, nao mais aqui.
    return Materias::Carregadas().Desbloqueada(static_cast<int>(subject), ProgressoDoMapa());
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

void Game::EditarPersonagem(const size_t posicao) {
    if (posicao >= mSessoes.size()) return;
    mPosicaoAntesDaEdicao = mPosicaoEntrando;
    mPosicaoEntrando = posicao;
    mEditandoPersonagem = true;
    RequestSceneChange(Scene::SceneType::CriacaoDePersonagem);
}

void Game::TerminarEdicaoDePersonagem() {
    mPosicaoEntrando = mPosicaoAntesDaEdicao;
    mEditandoPersonagem = false;
    RequestSceneChange(Scene::SceneType::EscolhaDaDificuldade);
}

float Game::RecordeDaPartida(const int materia) const {
    const auto modo = QuantosJogadores() > 1 ? Progresso::Modo::Dupla : Progresso::Modo::Solo;
    return ProgressoDoMapa().Recorde(materia, modo, NivelDeDificuldade()).nota;
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
