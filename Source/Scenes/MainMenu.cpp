//
// Created by nslop on 23/07/2024.
//

#include "MainMenu.h"
#include "../Painel.h"
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

        /// Quantos jogam. SO VALE quando o destino e a tela de matricula: e ali
        /// que uma partida comeca.
        ///
        /// O campo existe em TODAS as linhas de proposito - assim nenhuma opcao
        /// nova pode esquecer de dizer quantos jogam e herdar o numero da
        /// anterior.
        Sessao::Modo jogadores = Sessao::Modo::Um;
    };

    /// AS OPCOES DO MENU, numa tabela. Acrescentar uma e acrescentar uma linha:
    /// a navegacao, o desenho e o posicionamento saem todos do tamanho desta
    /// lista, e nao de numeros repetidos em tres lugares.
    ///
    /// ERAM CINCO, E ELAS SE SOBREPUNHAM NA TELA: "Carregar Perfil" saia
    /// impresso por cima de "Dois Jogadores". O menu escolhia o passo pela
    /// quantidade, mas o tamanho da letra e a altura da moldura continuavam
    /// constantes - entao apertar o passo nao apertava nada. As duas coisas
    /// foram consertadas: a lista encolheu e o tamanho agora SAI do passo.
    ///
    /// NOVO JOGO E CARREGAR PERFIL SUMIRAM, e com eles o Game::ModoDeEntrada.
    /// A tela de matricula descobre sozinha quem ja tem perfil: ha arquivo,
    /// abre; nao ha, cria. Perguntar antes obrigava o jogador a saber a
    /// resposta de uma pergunta que a propria matricula responde - e numa dupla
    /// obrigava os dois ao mesmo caminho.
    const Opcao kOpcoes[] = {
        {"Modo Solo",   Scene::SceneType::Identificacao, Sessao::Modo::Um},
        {"Modo Dupla",  Scene::SceneType::Identificacao, Sessao::Modo::Dois},
        {"Ranking",     Scene::SceneType::Ranking,       Sessao::Modo::Um},
        {"Opcoes",      Scene::SceneType::Opcoes,        Sessao::Modo::Um},
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
    // A FAIXA FOI MEDIDA NA TELA, e nao escolhida no escuro.
    //
    // O comentario antigo dizia que a arte do titulo ocupa ate y = 607. Fui
    // conferir fotografando o menu e procurando o ultimo pixel branco da arte
    // FORA da placa: ela acaba em 571 de 800, ou seja 0,714 - trinta e seis
    // pixels acima do que estava escrito, e ninguem tinha notado porque o
    // numero so servia para escolher estes dois.
    //
    // Com a faixa comecando em 0,775 sobram vinte pixels de respiro entre o
    // "Bullet Hell" e a placa, treze entre a placa e o fundo da tela, e o passo
    // fica em 46 px. Nao da para subir muito mais: a 0,76 a placa encosta na
    // arte, e a 0,79 (o valor antigo) a letra cai no minimo legivel.
    constexpr float kTopoDasOpcoes  = 0.775f;   // logo abaixo da arte do titulo
    constexpr float kFundoDasOpcoes = 0.948f;   // com folga para a moldura e a placa

    // A moldura e larga o bastante para a maior opcao, e fixa: uma moldura que
    // mudasse de largura a cada passo chamaria mais atencao que a propria escolha.
    constexpr int kSelecaoLargura = 460;

    /// O TETO da moldura e da letra. A ALTURA DE VERDADE sai do passo - ver
    /// AlturaDaSelecao. Eram constantes, e e por isso que cinco opcoes se
    /// sobrepuseram: o passo encolheu de 54 para 33 e a moldura continuou 54.
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

    /// Quantos pixels separam uma opcao da seguinte.
    float PassoEmPixels(const float alturaDaTela) {
        if (kQuantasOpcoes <= 1) return alturaDaTela * (kFundoDasOpcoes - kTopoDasOpcoes);
        return alturaDaTela * (kFundoDasOpcoes - kTopoDasOpcoes)
             / static_cast<float>(kQuantasOpcoes - 1);
    }

    /**
     * @brief A MOLDURA NUNCA PASSA DO PASSO.
     *
     * Era uma constante de 54 ao lado de um passo que encolhe com a quantidade.
     * O comentario da faixa dizia que acrescentar uma opcao "aperta as quatro
     * em vez de empurrar alguem para fora" - e o passo apertava mesmo, mas a
     * moldura e a letra nao. Com cinco opcoes o passo caiu para 33 px e as
     * molduras de 54 passaram a se sobrepor: "Carregar Perfil" saia impresso
     * por cima de "Dois Jogadores", e nada avisou.
     *
     * Agora o tamanho SAI do passo. Uma sexta opcao encolhe a letra ate o
     * minimo legivel; passado dele, a faixa e que tem de crescer, e ai o menu
     * nao cabe mais em silencio - ele fica pequeno demais e da para ver.
     */
    int AlturaDaSelecao(const float alturaDaTela) {
        const int doPasso = static_cast<int>(PassoEmPixels(alturaDaTela)) - kFolgaEntreMolduras;
        if (doPasso >= kSelecaoAlturaMax) return kSelecaoAlturaMax;
        return doPasso > kTamanhoMinimo + kFolgaDoTexto ? doPasso
                                                        : kTamanhoMinimo + kFolgaDoTexto;
    }

    /// E A LETRA CABE NA MOLDURA, pelo mesmo motivo.
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
    // Ver Documentacao/requisitos-coop.md, RF1.
    const bool cima  = Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Direcao::Cima);
    const bool baixo = Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Direcao::Baixo);

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

    // O BOTAO 1 do painel e o espaco, entao quem ja conhece o jogo nao perde
    // nada. O ENTER fica como atalho de teclado, sem aparecer escrito.
    const bool confirmar = Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Botao::Um)
                        || keyState[SDL_SCANCODE_RETURN]
                        || keyState[SDL_SCANCODE_KP_ENTER];

    if (confirmar && !mConfirmarAnterior) {
        mConfirmarAnterior = true;

        // SO QUEM COMECA UMA PARTIDA MEXE NAS SESSOES. Definir o modo de jogo
        // REFAZ as sessoes, e abrir o ranking ou as opcoes nao e comecar
        // partida nenhuma: zerar ali apagaria quem esta jogando, e a linha
        // "voce esta em 23o" sumiria do ranking de quem acabou de abri-lo.
        if (kOpcoes[mSelecionada].destino == Scene::SceneType::Identificacao) {
            mGame->DefinirModoDeJogo(kOpcoes[mSelecionada].jogadores);
        }

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
                                            static_cast<int>(ultima - primeira)
                                                + AlturaDaSelecao(altura)
                                                + 2 * kMargemDaPlaca,
                                            kSemBorda, kFundoPlaca, 0, 85);
    AddActor(std::move(placa));

    // A moldura entra ANTES dos textos e com ordem de desenho menor, para ficar
    // atras deles - Scene::AddDrawable ordena crescente, entao quem tem numero
    // menor desenha primeiro.
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
