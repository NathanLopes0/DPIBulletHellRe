//
// Created by nslop on 23/07/2024.
//

#include "MainMenu.h"
#include "../Game.h"
#include "../Actors/Actor.h"
#include "../Font.h"
#include "../Components/DrawComponents/DrawCaixaComponent.h"
#include "../Components/DrawComponents/DrawTextComponent.h"
#include "../Components/DrawComponents/DrawSpriteComponent.h"

#include "../CaminhosArquivo.h"

namespace {

    struct Opcao {
        const char* texto;
        Scene::SceneType destino;

        /// So vale quando o destino e a tela de matricula; ver Game::ModoDeEntrada.
        Game::ModoDeEntrada modo;
    };

    /// AS OPCOES DO MENU, numa tabela. Acrescentar uma e acrescentar uma linha:
    /// a navegacao, o desenho e o posicionamento saem todos do tamanho desta
    /// lista, e nao de numeros repetidos em tres lugares.
    ///
    /// NOVO JOGO e CARREGAR PERFIL levam a MESMA tela, em modos diferentes: uma
    /// copia da tela por modo divergiria na primeira correcao feita em so uma
    /// delas.
    const Opcao kOpcoes[] = {
        {"Novo Jogo",       Scene::SceneType::Identificacao, Game::ModoDeEntrada::Novo},
        {"Carregar Perfil", Scene::SceneType::Identificacao, Game::ModoDeEntrada::Carregar},
        {"Ranking",         Scene::SceneType::Ranking,       Game::ModoDeEntrada::Novo},
        {"Opcoes",          Scene::SceneType::Opcoes,        Game::ModoDeEntrada::Novo},
    };
    constexpr int kQuantasOpcoes = static_cast<int>(sizeof(kOpcoes) / sizeof(kOpcoes[0]));

    // A arte do titulo ocupa ate y = 607 numa imagem de 800 de altura, entao as
    // opcoes vivem na faixa de baixo. Em fracao da altura, e nao em pixels, para
    // a tela aguentar outra resolucao.
    // A FAIXA ONDE AS OPCOES CABEM, e nao um espacamento fixo.
    //
    // Antes havia um passo constante de 0,067 calculado para TRES opcoes. Ao
    // entrar a quarta - o Ranking - a ultima caiu para 1,009 da altura, ou seja,
    // para fora da tela, e nada avisou: o menu continuou funcionando, com uma
    // opcao que so dava para escolher as cegas.
    //
    // Agora o passo SAI da faixa e da quantidade. Acrescentar uma quinta aperta
    // as quatro em vez de empurrar alguem para fora.
    constexpr float kTopoDasOpcoes  = 0.790f;   // logo abaixo da arte do titulo
    constexpr float kFundoDasOpcoes = 0.955f;   // com folga para a moldura

    constexpr int kTamanhoDaOpcao = 40;

    // A moldura e larga o bastante para a maior opcao, e fixa: uma moldura que
    // mudasse de largura a cada passo chamaria mais atencao que a propria escolha.
    constexpr int kSelecaoLargura = 460;
    constexpr int kSelecaoAltura  = 54;

    /// Sobra de placa em volta das opcoes, em cima e embaixo.
    constexpr int kMargemDaPlaca = 10;

    constexpr SDL_Color kBordaDaSelecao = {235, 235, 235, 255};
    constexpr SDL_Color kFundoDaSelecao = { 34,  38,  48, 150};

    /// A PLACA ATRAS DAS OPCOES existe por legibilidade, nao por enfeite: o fundo
    /// do menu e uma foto do corredor do departamento, clara e cheia de detalhe,
    /// e texto branco solto em cima dela some em metade da largura. Sem moldura,
    /// para nao competir com a moldura da opcao em foco.
    constexpr SDL_Color kSemBorda   = {0, 0, 0, 0};
    constexpr SDL_Color kFundoPlaca = {12, 13, 17, 185};

    float AlturaDaOpcao(const int i, const float alturaDaTela) {

        // Uma opcao so fica no meio da faixa: dividir por zero poria o menu no
        // infinito, e encostar no topo nao seria o centro de nada.
        if (kQuantasOpcoes <= 1) {
            return alturaDaTela * (kTopoDasOpcoes + kFundoDasOpcoes) / 2.0f;
        }

        const float passo = (kFundoDasOpcoes - kTopoDasOpcoes)
                          / static_cast<float>(kQuantasOpcoes - 1);
        return alturaDaTela * (kTopoDasOpcoes + static_cast<float>(i) * passo);
    }
}

MainMenu::MainMenu(Game* game)
    : Scene(game, SceneType::MainMenu)
    , mMainMenuFont(std::make_unique<Font>())
{
    mMainMenuFont->Load(Caminhos::Asset("Fonts/Zelda.ttf"));
}

void MainMenu::Load() {

    LoadBackground();
    LoadTitle();
    CriarOpcoes();
    PosicionarSelecao();

}

void MainMenu::OnProcessInput(const Uint8 *keyState) {

    const bool cima  = keyState[SDL_SCANCODE_UP]   || keyState[SDL_SCANCODE_W];
    const bool baixo = keyState[SDL_SCANCODE_DOWN] || keyState[SDL_SCANCODE_S];

    if (cima && !mCimaAnterior) {
        // Soma kQuantasOpcoes antes do resto: em C++ o resto de um negativo e
        // negativo, e -1 % 3 daria -1 em vez de dar a volta para a ultima opcao.
        mSelecionada = (mSelecionada - 1 + kQuantasOpcoes) % kQuantasOpcoes;
        PosicionarSelecao();
    }
    mCimaAnterior = cima;

    if (baixo && !mBaixoAnterior) {
        mSelecionada = (mSelecionada + 1) % kQuantasOpcoes;
        PosicionarSelecao();
    }
    mBaixoAnterior = baixo;

    // ESPACO continua valendo junto com ENTER: era a unica tecla do menu antigo,
    // e quem ja conhece o jogo vai tentar ela primeiro.
    const bool confirmar = keyState[SDL_SCANCODE_RETURN]
                        || keyState[SDL_SCANCODE_KP_ENTER]
                        || keyState[SDL_SCANCODE_SPACE];

    if (confirmar && !mConfirmarAnterior) {
        mConfirmarAnterior = true;
        mGame->DefinirModoDeEntrada(kOpcoes[mSelecionada].modo);

        // O ranking aberto pelo menu e o GERAL - a media do curso. O de uma
        // materia so se chega pela selecao de fases, com a materia em foco.
        if (kOpcoes[mSelecionada].destino == Scene::SceneType::Ranking) {
            mGame->PedirRanking(Game::PedidoDeRanking{true, 0, Scene::SceneType::MainMenu});
        }

        mGame->RequestSceneChange(kOpcoes[mSelecionada].destino);
        return;
    }
    mConfirmarAnterior = confirmar;
}

void MainMenu::OnUpdate(float deltaTime) {

}

void MainMenu::LoadBackground() {
    auto background = std::make_unique<Actor>(this);
    background->SetPosition(Vector2(mGame->GetWindowWidth() / 2.0f, mGame->GetWindowHeight() / 2.0f));
    background->AddComponent<DrawSpriteComponent>(Caminhos::Asset("MainMenuBackground.png"), 50);

    mBackgroundActor = background.get(); //Guarda o ponteiro observador
    AddActor(std::move(background));
}
void MainMenu::LoadTitle() {
    auto title = std::make_unique<Actor>(this);
    title->SetPosition(Vector2(static_cast<float>(mGame->GetWindowWidth()) / 2.0f,
                                    static_cast<float>(mGame->GetWindowHeight()) / 2.2f));

    title->AddComponent<DrawSpriteComponent>(Caminhos::Asset("DPIBHTitleMainMenu.png"), 75);

    mTitleActor = title.get();
    AddActor(std::move(title));
}

void MainMenu::CriarOpcoes() {

    const auto largura = static_cast<float>(mGame->GetWindowWidth());
    const auto altura  = static_cast<float>(mGame->GetWindowHeight());

    // A placa cobre da primeira a ultima opcao, com margem. A conta sai das
    // mesmas constantes das linhas, entao mudar o espacamento ou acrescentar uma
    // opcao ajusta a placa sozinho.
    const float primeira = AlturaDaOpcao(0, altura);
    const float ultima   = AlturaDaOpcao(kQuantasOpcoes - 1, altura);

    auto placa = std::make_unique<Actor>(this);
    placa->SetPosition(Vector2(largura / 2.0f, (primeira + ultima) / 2.0f));
    placa->AddComponent<DrawCaixaComponent>(kSelecaoLargura + 60,
                                            static_cast<int>(ultima - primeira) + kSelecaoAltura
                                                + 2 * kMargemDaPlaca,
                                            kSemBorda, kFundoPlaca, 0, 85);
    AddActor(std::move(placa));

    // A moldura entra ANTES dos textos e com ordem de desenho menor, para ficar
    // atras deles - Scene::AddDrawable ordena crescente, entao quem tem numero
    // menor desenha primeiro.
    auto selecao = std::make_unique<Actor>(this);
    selecao->SetPosition(Vector2(largura / 2.0f, AlturaDaOpcao(0, altura)));
    selecao->AddComponent<DrawCaixaComponent>(kSelecaoLargura, kSelecaoAltura,
                                              kBordaDaSelecao, kFundoDaSelecao, 3, 90);
    mSelecaoAtor = selecao.get();
    AddActor(std::move(selecao));

    for (int i = 0; i < kQuantasOpcoes; ++i) {

        auto ator = std::make_unique<Actor>(this);
        ator->SetPosition(Vector2(largura / 2.0f, AlturaDaOpcao(i, altura)));

        auto dc = ator->AddComponent<DrawTextComponent>(kOpcoes[i].texto, mMainMenuFont.get(),
                                                        kSelecaoLargura - 40, kTamanhoDaOpcao + 6,
                                                        kTamanhoDaOpcao, 100);
        dc->SetLarguraDeQuebra(kSelecaoLargura);
        // Sem esticar: a caixa e limite, nao destino. Ver DrawTextComponent.
        dc->SetAjustarAoTexto(true);

        mOpcaoAtores.push_back(ator.get());
        AddActor(std::move(ator));
    }
}

void MainMenu::PosicionarSelecao() const {

    if (!mSelecaoAtor) return;

    const auto largura = static_cast<float>(mGame->GetWindowWidth());
    const auto altura  = static_cast<float>(mGame->GetWindowHeight());

    mSelecaoAtor->SetPosition(Vector2(largura / 2.0f, AlturaDaOpcao(mSelecionada, altura)));
}
