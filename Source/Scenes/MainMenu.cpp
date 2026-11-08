//
// Created by nslop on 23/07/2024.
//

#include "MainMenu.h"
#include "../Entrada/Painel.h"
#include "../Motor/Game.h"
#include "../Actors/Actor.h"
#include "../Motor/Font.h"
#include "../Components/DrawComponents/DrawCaixaComponent.h"
#include "../Components/DrawComponents/DrawTextComponent.h"
#include "../Components/DrawComponents/DrawSpriteComponent.h"

#include "../Motor/CaminhosArquivo.h"

namespace {

    struct Opcao {
        const char* texto;
        Scene::SceneType destino;

        /// Quantos jogam. So vale quando o destino e a tela de matricula: e ali que uma partida
        /// comeca.
        Sessao::Modo jogadores = Sessao::Modo::Um;
    };

    /// As opcoes do menu, numa tabela.
    const Opcao kOpcoes[] = {
        {"Modo Solo",   Scene::SceneType::Identificacao, Sessao::Modo::Um},
        {"Modo Dupla",  Scene::SceneType::Identificacao, Sessao::Modo::Dois},
        {"Como Jogar",  Scene::SceneType::ComoJogar,     Sessao::Modo::Um},
        {"Ranking",     Scene::SceneType::Ranking,       Sessao::Modo::Um},
        {"Opcoes",      Scene::SceneType::Opcoes,        Sessao::Modo::Um},
    };
    constexpr int kQuantasOpcoes = static_cast<int>(sizeof(kOpcoes) / sizeof(kOpcoes[0]));

    // A arte do titulo ocupa ate y = 607 numa imagem de 800 de altura, entao as opcoes vivem na
    // faixa de baixo.
    constexpr float kTopoDasOpcoes  = 0.775f;   // logo abaixo da arte do titulo
    constexpr float kFundoDasOpcoes = 0.948f;   // com folga para a moldura e a placa

    // A moldura e larga o bastante para a maior opcao, e fixa: uma moldura que
    // mudasse de largura a cada passo chamaria mais atencao que a propria escolha.
    constexpr int kSelecaoLargura = 460;

    /// O teto da moldura e da letra. A altura de verdade sai do passo - ver AlturaDaSelecao.
    constexpr int kSelecaoAlturaMax = 54;
    constexpr int kTamanhoMaximo = 40;

    /// Abaixo disto a opcao deixa de ser legivel do outro lado do gabinete, e e
    /// melhor a lista encolher do que a letra sumir.
    constexpr int kTamanhoMinimo = 22;

    /// Respiro entre uma moldura e a seguinte, e entre a letra e a moldura.
    constexpr int kFolgaEntreMolduras = 8;
    constexpr int kFolgaDoTexto = 14;

    /// Sobra de placa em volta das opcoes, em cima e embaixo.
    constexpr int kMargemDaPlaca = 10;

    constexpr SDL_Color kBordaDaSelecao = {235, 235, 235, 255};
    constexpr SDL_Color kFundoDaSelecao = { 34,  38,  48, 150};

    /// A placa atras das opcoes existe por legibilidade, nao por enfeite. Sem moldura, para nao
    /// competir com a moldura da opcao em foco.
    constexpr SDL_Color kSemBorda   = {0, 0, 0, 0};
    constexpr SDL_Color kFundoPlaca = {12, 13, 17, 185};

    float AlturaDaOpcao(const int i, const float alturaDaTela) {

        // Uma opcao so fica no meio da faixa.
        if (kQuantasOpcoes <= 1) {
            return alturaDaTela * (kTopoDasOpcoes + kFundoDasOpcoes) / 2.0f;
        }

        const float passo = (kFundoDasOpcoes - kTopoDasOpcoes)
                          / static_cast<float>(kQuantasOpcoes - 1);
        return alturaDaTela * (kTopoDasOpcoes + static_cast<float>(i) * passo);
    }

    /// Quantos pixels separam uma opcao da seguinte.
    float PassoEmPixels(const float alturaDaTela) {
        if (kQuantasOpcoes <= 1) return alturaDaTela * (kFundoDasOpcoes - kTopoDasOpcoes);
        return alturaDaTela * (kFundoDasOpcoes - kTopoDasOpcoes)
             / static_cast<float>(kQuantasOpcoes - 1);
    }

    /// A moldura nunca passa do passo.
    int AlturaDaSelecao(const float alturaDaTela) {
        const int doPasso = static_cast<int>(PassoEmPixels(alturaDaTela)) - kFolgaEntreMolduras;
        if (doPasso >= kSelecaoAlturaMax) return kSelecaoAlturaMax;
        return doPasso > kTamanhoMinimo + kFolgaDoTexto ? doPasso
                                                        : kTamanhoMinimo + kFolgaDoTexto;
    }

    /// E a letra cabe na moldura, pelo mesmo motivo.
    int TamanhoDaOpcao(const float alturaDaTela) {
        const int daMoldura = AlturaDaSelecao(alturaDaTela) - kFolgaDoTexto;
        if (daMoldura > kTamanhoMaximo) return kTamanhoMaximo;
        return daMoldura < kTamanhoMinimo ? kTamanhoMinimo : daMoldura;
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

    // Quem escolhe no menu e o jogador 1, inclusive a opcao de dois jogadores.
    const bool cima  = Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Direcao::Cima);
    const bool baixo = Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Direcao::Baixo);

    if (cima && !mCimaAnterior) {
        // Soma kQuantasOpcoes antes do resto.
        mSelecionada = (mSelecionada - 1 + kQuantasOpcoes) % kQuantasOpcoes;
        PosicionarSelecao();
    }
    mCimaAnterior = cima;

    if (baixo && !mBaixoAnterior) {
        mSelecionada = (mSelecionada + 1) % kQuantasOpcoes;
        PosicionarSelecao();
    }
    mBaixoAnterior = baixo;

    // O botao 1 do painel e o espaco, entao quem ja conhece o jogo nao perde nada.
    const bool confirmar = Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Botao::Um)
                        || keyState[SDL_SCANCODE_RETURN]
                        || keyState[SDL_SCANCODE_KP_ENTER];

    if (confirmar && !mConfirmarAnterior) {
        mConfirmarAnterior = true;

        // So quem comeca uma partida mexe nas sessoes.
        if (kOpcoes[mSelecionada].destino == Scene::SceneType::Identificacao) {
            mGame->DefinirModoDeJogo(kOpcoes[mSelecionada].jogadores);
        }

        // O ranking aberto pelo menu e o geral - a media do curso.
        if (kOpcoes[mSelecionada].destino == Scene::SceneType::Ranking) {
            mGame->PedirRanking(Game::PedidoDeRanking{true, 0, Scene::SceneType::MainMenu});
        }

        mGame->RequestSceneChange(kOpcoes[mSelecionada].destino);
        return;
    }
    mConfirmarAnterior = confirmar;
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

    // A placa cobre da primeira a ultima opcao, com margem.
    const float primeira = AlturaDaOpcao(0, altura);
    const float ultima   = AlturaDaOpcao(kQuantasOpcoes - 1, altura);

    auto placa = std::make_unique<Actor>(this);
    placa->SetPosition(Vector2(largura / 2.0f, (primeira + ultima) / 2.0f));
    placa->AddComponent<DrawCaixaComponent>(kSelecaoLargura + 60,
                                            static_cast<int>(ultima - primeira)
                                                + AlturaDaSelecao(altura)
                                                + 2 * kMargemDaPlaca,
                                            kSemBorda, kFundoPlaca, 0, 85);
    AddActor(std::move(placa));

    // A moldura entra antes dos textos e com ordem de desenho menor, para ficar atras deles.
    auto selecao = std::make_unique<Actor>(this);
    selecao->SetPosition(Vector2(largura / 2.0f, AlturaDaOpcao(0, altura)));
    selecao->AddComponent<DrawCaixaComponent>(kSelecaoLargura, AlturaDaSelecao(altura),
                                              kBordaDaSelecao, kFundoDaSelecao, 3, 90);
    mSelecaoAtor = selecao.get();
    AddActor(std::move(selecao));

    for (int i = 0; i < kQuantasOpcoes; ++i) {

        auto ator = std::make_unique<Actor>(this);
        ator->SetPosition(Vector2(largura / 2.0f, AlturaDaOpcao(i, altura)));

        auto dc = ator->AddComponent<DrawTextComponent>(kOpcoes[i].texto, mMainMenuFont.get(),
                                                        kSelecaoLargura - 40,
                                                        TamanhoDaOpcao(altura) + 6,
                                                        TamanhoDaOpcao(altura), 100);
        dc->SetLarguraDeQuebra(kSelecaoLargura);
        // Sem esticar: a caixa e limite, nao destino.
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
