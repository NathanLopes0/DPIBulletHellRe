//
// Created by nslop on 23/07/2024.
//

#include "MainMenu.h"
#include "../Entrada/Painel.h"
#include "../Motor/Game.h"
#include "../Motor/AudioSystem.h"
#include "../Actors/Actor.h"
#include "../Motor/Font.h"
#include "../Components/DrawComponents/DrawCaixaComponent.h"
#include "../Components/DrawComponents/DrawTextComponent.h"
#include "../Components/DrawComponents/DrawSpriteComponent.h"

#include "../Motor/CaminhosArquivo.h"
#include "../Motor/Fontes.h"
#include "TextoDeCena.h"

#include <cmath>

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

    /// O Modo Dupla em foco: dourado, e o fundo um pouco mais quente. O pulso so mexe na moldura.
    constexpr SDL_Color kBordaDaDupla = {255, 214, 90, 255};
    constexpr SDL_Color kFundoDaDupla = { 58,  50,  30, 170};
    constexpr float kPulsoPorSegundo = 0.8f;

    /// A dica do botao, a direita da moldura.
    constexpr int kTamanhoDaDica = 20;
    constexpr int kLarguraDaDica = 300;
    constexpr float kVaoDaDica = 24.0f;

    /// A placa atras das opcoes existe por legibilidade, nao por enfeite. Sem moldura, para nao
    /// competir com a moldura da opcao em foco.
    /// O aviso de que o codigo secreto entrou, e por quanto tempo ele fica.
    constexpr const char* kAvisoDoSegredo = "Um segredo te espera depois da INF 213...";
    constexpr float kTempoDoAviso = 3.5f;
    constexpr int kLarguraDoAviso = 900;
    constexpr SDL_Color kBordaDoAviso = {255, 209, 77, 255};

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
    mMainMenuFont->Load(Caminhos::Asset(Fontes::kJogo));
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

    // O codigo le as mesmas setas antes do menu andar (as bordas sao dele).
    const bool fechouOCodigo = TocarNoCodigo(keyState, cima, baixo);

    if (mBordaCima.Bateu(cima)) {
        // Soma kQuantasOpcoes antes do resto.
        mSelecionada = (mSelecionada - 1 + kQuantasOpcoes) % kQuantasOpcoes;
        PosicionarSelecao();
    }

    if (mBordaBaixo.Bateu(baixo)) {
        mSelecionada = (mSelecionada + 1) % kQuantasOpcoes;
        PosicionarSelecao();
    }

    // O botao 1 do painel e o espaco, entao quem ja conhece o jogo nao perde nada.
    const bool confirmar = Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Botao::Um)
                        || keyState[SDL_SCANCODE_RETURN]
                        || keyState[SDL_SCANCODE_KP_ENTER];

    // O botao que fecha o codigo secreto nao escolhe a opcao em foco.
    if (mBordaConfirmar.Bateu(confirmar) && !fechouOCodigo) {

        // So quem comeca uma partida mexe nas sessoes.
        if (kOpcoes[mSelecionada].destino == Scene::SceneType::Identificacao) {
            mGame->DefinirModoDeJogo(kOpcoes[mSelecionada].jogadores);
        }

        // O ranking aberto pelo menu e o geral - a media do curso.
        if (kOpcoes[mSelecionada].destino == Scene::SceneType::Ranking) {
            mGame->PedirRanking(Game::PedidoDeRanking{true, 0, Scene::SceneType::MainMenu});
        }

        if (kOpcoes[mSelecionada].destino == Scene::SceneType::Opcoes) {
            mGame->AbrirOpcoes();
            return;
        }

        mGame->RequestSceneChange(kOpcoes[mSelecionada].destino);
        return;
    }
}

bool MainMenu::TocarNoCodigo(const Uint8* keyState, const bool cima, const bool baixo) {
    using CodigoSecreto::Toque;
    const Painel::Jogador um = Painel::Jogador::Um;
    bool fechou = false;
    const auto tocar = [&](const Toque t) { if (mCodigo.Tocar(t)) fechou = true; };
    if (mBordaCimaDoCodigo.Bateu(cima)) tocar(Toque::Cima);
    if (mBordaBaixoDoCodigo.Bateu(baixo)) tocar(Toque::Baixo);
    if (mBordaEsquerda.Bateu(Painel::Apertado(keyState, um, Painel::Direcao::Esquerda))) tocar(Toque::Esquerda);
    if (mBordaDireita.Bateu(Painel::Apertado(keyState, um, Painel::Direcao::Direita))) tocar(Toque::Direita);
    // O B e o botao 2 (no teclado, a tecla B); o A, o botao 1 - e a tecla A, de quem digita.
    if (mBordaB.Bateu(Painel::Apertado(keyState, um, Painel::Botao::Dois))) tocar(Toque::B);
    if (mBordaA.Bateu(Painel::Apertado(keyState, um, Painel::Botao::Um) || keyState[SDL_SCANCODE_A] != 0)) {
        tocar(Toque::A);
    }
    if (fechou) MostrarOSegredo();
    return fechou;
}

void MainMenu::MostrarOSegredo() {
    mGame->AceitarCodigoSecreto();
    if (auto* audio = mGame->GetAudio()) audio->PlaySound("powerup.wav", false);
    mTempoDoAviso = kTempoDoAviso;
    if (mAvisoDoSegredo) mAvisoDoSegredo->SetIsVisible(true);
    if (mFaixaDoSegredo) mFaixaDoSegredo->SetIsVisible(true);
    SDL_Log("MENU: o codigo secreto entrou.");
}

void MainMenu::OnUpdate(const float deltaTime) {
    mPulso += deltaTime;
    PintarSelecao();
    if (mTempoDoAviso <= 0.0f || !mAvisoDoSegredo) return;
    mTempoDoAviso -= deltaTime;
    // Pisca entre o dourado e o ciano da revelacao, e some no fim.
    const bool dourado = static_cast<int>(mTempoDoAviso * 6.0f) % 2 == 0;
    mAvisoDoSegredo->SetColor(dourado ? Vector3(1.0f, 0.82f, 0.3f) : Vector3(0.45f, 0.9f, 1.0f));
    if (mTempoDoAviso <= 0.0f) {
        mAvisoDoSegredo->SetIsVisible(false);
        if (mFaixaDoSegredo) mFaixaDoSegredo->SetIsVisible(false);
    }
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

    // A dica do botao 1, que anda com a moldura (PosicionarSelecao).
    mDicaAtor = CriarTexto(*this, mMainMenuFont.get(),
                           Painel::Rodape({{Painel::Jogador::Um, Painel::Botao::Um, "confirmar"}}, mGame->JeitoDoRodape()),
                           Vector2(largura / 2.0f, AlturaDaOpcao(0, altura)), kTamanhoDaDica, kLarguraDaDica,
                           kTamanhoDaDica + 8)->GetOwner();
    // Sobre a foto do fundo, uma placa como a das opcoes, para a letra nao sumir na mesa.
    mDicaAtor->AddComponent<DrawCaixaComponent>(kLarguraDaDica, kTamanhoDaDica + 16, kSemBorda, kFundoPlaca, 0, 85);

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

    // O aviso do codigo secreto, numa faixa escura no alto da tela: so aparece quando o codigo entra.
    auto aviso = std::make_unique<Actor>(this);
    aviso->SetPosition(Vector2(largura / 2.0f, altura * 0.06f));
    mFaixaDoSegredo = aviso->AddComponent<DrawCaixaComponent>(kLarguraDoAviso + 60, 64, kBordaDoAviso, kFundoPlaca, 2, 105);
    mFaixaDoSegredo->SetIsVisible(mGame->CodigoSecretoAceito());
    mAvisoDoSegredo = aviso->AddComponent<DrawTextComponent>(kAvisoDoSegredo, mMainMenuFont.get(),
                                                             kLarguraDoAviso, 48, 38, 110);
    mAvisoDoSegredo->SetLarguraDeQuebra(kLarguraDoAviso);
    mAvisoDoSegredo->SetAjustarAoTexto(true);
    mAvisoDoSegredo->SetColor(Vector3(1.0f, 0.82f, 0.3f));
    mAvisoDoSegredo->SetIsVisible(mGame->CodigoSecretoAceito());
    if (mGame->CodigoSecretoAceito()) mTempoDoAviso = kTempoDoAviso;
    AddActor(std::move(aviso));
}

void MainMenu::PosicionarSelecao() const {

    if (!mSelecaoAtor) return;

    const auto largura = static_cast<float>(mGame->GetWindowWidth());
    const auto altura  = static_cast<float>(mGame->GetWindowHeight());

    mSelecaoAtor->SetPosition(Vector2(largura / 2.0f, AlturaDaOpcao(mSelecionada, altura)));
    if (mDicaAtor) {
        const float x = largura / 2.0f + static_cast<float>(kSelecaoLargura) / 2.0f + kVaoDaDica +
                        static_cast<float>(kLarguraDaDica) / 2.0f;
        mDicaAtor->SetPosition(Vector2(x, AlturaDaOpcao(mSelecionada, altura)));
    }
}

void MainMenu::PintarSelecao() {
    const bool dupla = kOpcoes[mSelecionada].jogadores == Sessao::Modo::Dois;
    if (auto* caixa = mSelecaoAtor ? mSelecaoAtor->GetComponent<DrawCaixaComponent>() : nullptr) {
        SDL_Color borda = dupla ? kBordaDaDupla : kBordaDaSelecao;
        if (dupla) {
            // Respira devagar entre o dourado cheio e um pouco apagado.
            const float u = 0.5f + 0.5f * std::sin(mPulso * 6.2831853f * kPulsoPorSegundo);
            borda.a = static_cast<Uint8>(160.0f + 95.0f * u);
        }
        caixa->SetMoldura(borda);
        caixa->SetFundo(dupla ? kFundoDaDupla : kFundoDaSelecao);
    }
    for (int i = 0; i < static_cast<int>(mOpcaoAtores.size()); ++i) {
        auto* dc = mOpcaoAtores[static_cast<size_t>(i)]->GetComponent<DrawTextComponent>();
        if (!dc) continue;
        const bool douradaAgora = i == mSelecionada && kOpcoes[i].jogadores == Sessao::Modo::Dois;
        dc->SetColor(douradaAgora ? Vector3(1.0f, 0.86f, 0.42f) : Vector3(1.0f, 1.0f, 1.0f));
    }
}
