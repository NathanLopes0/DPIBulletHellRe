//
// A tela "Como Jogar", do menu principal.
//

#include "TelaComoJogar.h"

#include <SDL_scancode.h>

#include "FundoDeCena.h"
#include "../Entrada/Painel.h"
#include "../Interface/ManualArquivo.h"
#include "../Motor/Game.h"
#include "../Motor/Font.h"
#include "../Motor/CaminhosArquivo.h"
#include "../Actors/Actor.h"
#include "../Components/DrawComponents/DrawCaixaComponent.h"
#include "../Components/DrawComponents/DrawTextComponent.h"

namespace {
    /// A PLACA atras do texto: a foto do corredor e clara e cheia de detalhe,
    /// e texto branco solto sobre ela some (FundoDeCena.h).
    constexpr int kLarguraDaPlaca = 1000;
    constexpr int kAlturaDaPlaca = 560;

    /// Em fracao da altura da tela, como as outras telas de menu.
    constexpr float kTopoDaPlaca = 0.155f;
    constexpr float kRodape = 0.93f;

    /// A partir do topo da placa, em pixels.
    constexpr float kTituloNaPlaca = 52.0f;
    constexpr float kPrimeiraLinha = 128.0f;
    constexpr float kEntreLinhas = 44.0f;
    constexpr float kIndicadorNaPlaca = 520.0f;
    constexpr float kSetas = 90.0f;   ///< do meio do indicador ao meio de cada seta

    /// Larga, para nenhuma linha quebrar: a que nao cabe encolhe a letra
    /// (SetAjustarAoTexto) em vez de virar duas.
    constexpr unsigned kSemQuebra = 4000;

    const SDL_Color kMolduraDaPlaca{120, 140, 180, 230};
    const SDL_Color kFundoDaPlaca{12, 13, 17, 215};
}

TelaComoJogar::TelaComoJogar(Game* game)
    : Scene(game, SceneType::ComoJogar),
      mFonte(std::make_unique<Font>())
{
    mFonte->Load(Caminhos::Asset("Fonts/Zelda.ttf"));
}

DrawTextComponent* TelaComoJogar::Texto(const std::string& conteudo, const float x, const float y,
                                        const int tamanho, const int larguraMaxima) {
    auto ator = std::make_unique<Actor>(this);
    ator->SetPosition(Vector2(x, y));
    // O texto vazio nao rasteriza: o que ainda nao tem conteudo nasce com um espaco.
    auto* dc = ator->AddComponent<DrawTextComponent>(conteudo.empty() ? " " : conteudo, mFonte.get(),
                                                     larguraMaxima, tamanho + 8, tamanho, 255);
    dc->SetLarguraDeQuebra(kSemQuebra);
    dc->SetAjustarAoTexto(true);
    AddActor(std::move(ator));
    return dc;
}

void TelaComoJogar::Load() {

    CriarFundoDoCorredor(*this);

    const auto largura = static_cast<float>(mGame->GetWindowWidth());
    const auto altura = static_cast<float>(mGame->GetWindowHeight());
    const float meio = largura / 2.0f;
    const float topo = altura * kTopoDaPlaca;

    Texto("COMO JOGAR", meio, topo / 2.0f, 52, 760);

    auto placa = std::make_unique<Actor>(this);
    placa->SetPosition(Vector2(meio, topo + kAlturaDaPlaca / 2.0f));
    placa->AddComponent<DrawCaixaComponent>(kLarguraDaPlaca, kAlturaDaPlaca, kMolduraDaPlaca, kFundoDaPlaca, 3, 100);
    AddActor(std::move(placa));

    mTitulo = Texto("", meio, topo + kTituloNaPlaca, 40, kLarguraDaPlaca - 60);
    mTitulo->SetColor(Color::Gold);
    for (int i = 0; i < Manual::kLinhasNoMaximo; ++i) {
        mLinhas.push_back(Texto("", meio, topo + kPrimeiraLinha + kEntreLinhas * static_cast<float>(i),
                                26, kLarguraDaPlaca - 60));
    }

    const float yIndicador = topo + kIndicadorNaPlaca;
    mIndicador = Texto("", meio, yIndicador, 24, 200);
    mSetaAnterior = Texto("<", meio - kSetas, yIndicador, 24, 40);
    mSetaProxima = Texto(">", meio + kSetas, yIndicador, 24, 40);

    mRodape = Texto("", meio, altura * kRodape, 24, 1000);

    Mostrar();
}

void TelaComoJogar::Mostrar() {
    const auto& paginas = Manual::Carregado().paginas;
    const int quantas = static_cast<int>(paginas.size());
    const bool arcade = mGame->NoGabinete();

    std::vector<std::string> linhas;
    if (quantas == 0) {
        // SEM O ARQUIVO, a tela diz o que houve em vez de ficar em branco.
        mTitulo->SetText("Manual indispon\xc3\xadvel");
        linhas = {"O arquivo Assets/como-jogar.json n\xc3\xa3o foi encontrado."};
        mIndicador->SetIsVisible(false);
    } else {
        const Manual::Pagina& p = paginas[static_cast<size_t>(mPagina)];
        mTitulo->SetText(p.titulo);
        linhas = Manual::Linhas(p, arcade);
        mIndicador->SetText(Manual::Indicador(mPagina, quantas));
        mIndicador->SetIsVisible(true);
    }
    for (size_t i = 0; i < mLinhas.size(); ++i) {
        const bool usada = i < linhas.size();
        mLinhas[i]->SetIsVisible(usada);
        if (usada) mLinhas[i]->SetText(linhas[i].empty() ? " " : linhas[i]);
    }

    const bool temAnterior = mPagina > 0;
    const bool temProxima = mPagina + 1 < quantas;
    mSetaAnterior->SetIsVisible(temAnterior);
    mSetaProxima->SetIsVisible(temProxima);

    // \xc3\xa1 e \xc3\xb3 sao o a e o o agudos: o MSVC nao le acento em literal.
    // NA ULTIMA PAGINA o botao 1 termina a leitura, e o rodape diz isso.
    mRodape->SetText(Painel::Rodape({Painel::Manche(Painel::Jogador::Um, "p\xc3\xa1ginas"),
                                     {Painel::Jogador::Um, Painel::Botao::Um,
                                      temProxima ? "pr\xc3\xb3xima" : "terminar"},
                                     {Painel::Jogador::Um, Painel::Botao::Dois, "voltar"}},
                                    mGame->JeitoDoRodape()));
}

void TelaComoJogar::OnProcessInput(const Uint8* keyState) {

    const bool voltar = Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Botao::Dois)
                     || keyState[SDL_SCANCODE_ESCAPE];
    if (voltar && !mVoltarAnterior) {
        mGame->RequestSceneChange(SceneType::MainMenu);
        return;
    }
    mVoltarAnterior = voltar;

    // QUALQUER UM DOS DOIS vira a pagina: a dupla le junta.
    bool esquerda = false, direita = false, avancar = false;
    for (const Painel::Jogador quem : Painel::kJogadores) {
        esquerda = esquerda || Painel::Apertado(keyState, quem, Painel::Direcao::Esquerda);
        direita = direita || Painel::Apertado(keyState, quem, Painel::Direcao::Direita);
        avancar = avancar || Painel::Apertado(keyState, quem, Painel::Botao::Um);
    }
    avancar = avancar || keyState[SDL_SCANCODE_RETURN] || keyState[SDL_SCANCODE_KP_ENTER];

    const int quantas = static_cast<int>(Manual::Carregado().paginas.size());
    int passo = 0;
    if (esquerda && !mEsquerdaAnterior) passo -= 1;
    if (direita && !mDireitaAnterior) passo += 1;
    const bool avancou = avancar && !mAvancarAnterior;
    mEsquerdaAnterior = esquerda;
    mDireitaAnterior = direita;
    mAvancarAnterior = avancar;

    if (avancou) {
        // NA ULTIMA (ou sem manual), o botao 1 termina: o leitor chegou ao fim.
        if (mPagina + 1 >= quantas) {
            mGame->RequestSceneChange(SceneType::MainMenu);
            return;
        }
        passo += 1;
    }

    const int nova = Manual::Virar(mPagina, passo, quantas);
    if (nova != mPagina) {
        mPagina = nova;
        Mostrar();
    }
}
