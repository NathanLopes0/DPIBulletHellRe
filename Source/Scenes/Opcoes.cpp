//
// A tela de opcoes.
//

#include "Opcoes.h"
#include "../Painel.h"
#include "FundoDeCena.h"

#include <SDL_scancode.h>

#include "../Game.h"
#include "../Font.h"
#include "../Configuracao.h"
#include "../CaminhosArquivo.h"
#include "../Actors/Actor.h"
#include "../Components/DrawComponents/DrawCaixaComponent.h"
#include "../Components/DrawComponents/DrawTextComponent.h"

namespace {
    // A LINHA DA OPCAO: uma moldura com o nome a esquerda e o valor a direita,
    // como as linhas do menu principal.
    constexpr int kLarguraDaLinha = 640;
    constexpr int kAlturaDaLinha = 56;
    constexpr float kRecuoDoTexto = 150.0f;   ///< do meio da linha ao meio de cada texto
    constexpr float kPrimeiraLinha = 0.30f;   ///< em fracao da altura
    constexpr float kEntreLinhas = 0.085f;

    const SDL_Color kMolduraEscolhida{255, 214, 90, 255};
    const SDL_Color kFundoEscolhido{58, 52, 34, 190};
    const SDL_Color kMoldura{235, 235, 235, 110};
    const SDL_Color kFundo{34, 38, 48, 150};

    // \xc3\xba e \xc3\xa3 sao u agudo e a til: o MSVC nao le acento em literal.
    const char* const kNomes[] = {"M\xc3\xbasica", "Efeitos", "Tela cheia", "Hitbox"};

    std::string Volume(const int v) {
        return v == 0 ? std::string("< mudo >") : "< " + std::to_string(v) + " >";
    }
    std::string SimOuNao(const bool b) { return b ? "< sim >" : "< n\xc3\xa3o >"; }
}

Opcoes::Opcoes(Game* game)
    : Scene(game, SceneType::Opcoes),
      mFonte(std::make_unique<Font>())
{
    mFonte->Load(Caminhos::Asset("Fonts/Zelda.ttf"));
}

void Opcoes::Load() {

    CriarFundoDoCorredor(*this);

    const auto largura = static_cast<float>(mGame->GetWindowWidth());
    const auto altura = static_cast<float>(mGame->GetWindowHeight());
    const float meio = largura / 2.0f;

    // \xc3\x87 e \xc3\x95 sao o C cedilha e o O til.
    Texto("OP\xc3\x87\xc3\x95" "ES", meio, altura * 0.15f, 52, 760);

    for (int k = 0; k < kItens; ++k) {
        const float y = altura * (kPrimeiraLinha + kEntreLinhas * static_cast<float>(k));
        auto linha = std::make_unique<Actor>(this);
        linha->SetPosition(Vector2(meio, y));
        mLinhas[k].caixa = linha->AddComponent<DrawCaixaComponent>(kLarguraDaLinha, kAlturaDaLinha,
                                                                    kMoldura, kFundo, 3, 100);
        AddActor(std::move(linha));

        Texto(kNomes[k], meio - kRecuoDoTexto, y, 30, 300);
        mLinhas[k].valor = Texto("", meio + kRecuoDoTexto, y, 30, 300)->GetComponent<DrawTextComponent>();
    }
    mDescricao = Texto("", meio, altura * 0.76f, 22, 760)->GetComponent<DrawTextComponent>();
    Mostrar();

    // O BOTAO 1 TAMBEM TROCA, sem aparecer escrito - como o ENTER: duas
    // entradas com o mesmo "trocar" no rodape pareciam duas acoes diferentes.
    Texto(Painel::Rodape({Painel::Manche(Painel::Jogador::Um, "escolher e trocar"),
                          {Painel::Jogador::Um, Painel::Botao::Dois, "voltar"}},
                         mGame->JeitoDoRodape()),
          meio, altura * 0.89f, 24, 760);
}

Actor* Opcoes::Texto(const std::string& conteudo, const float x, const float y, const int tamanho,
                     const int larguraMaxima) {

    auto ator = std::make_unique<Actor>(this);
    ator->SetPosition(Vector2(x, y));

    auto dc = ator->AddComponent<DrawTextComponent>(conteudo, mFonte.get(),
                                                    larguraMaxima, tamanho + 8, tamanho, 255);
    dc->SetLarguraDeQuebra(static_cast<unsigned>(larguraMaxima));
    dc->SetAjustarAoTexto(true);

    Actor* bruto = ator.get();
    AddActor(std::move(ator));
    return bruto;
}

void Opcoes::Mostrar() {
    for (int k = 0; k < kItens; ++k) {
        if (mLinhas[k].caixa) {
            mLinhas[k].caixa->SetMoldura(k == mEscolhida ? kMolduraEscolhida : kMoldura);
            mLinhas[k].caixa->SetFundo(k == mEscolhida ? kFundoEscolhido : kFundo);
        }
        DrawTextComponent* valor = mLinhas[k].valor;
        if (!valor) continue;
        valor->SetColor(Vector3(1.0f, 1.0f, 1.0f));
        switch (static_cast<Item>(k)) {
            case Item::Musica:    valor->SetText(Volume(mGame->VolumeDaMusica())); break;
            case Item::Efeitos:   valor->SetText(Volume(mGame->VolumeDosEfeitos())); break;
            case Item::TelaCheia: valor->SetText(SimOuNao(mGame->TelaCheia())); break;
            case Item::Hitbox:    valor->SetText(SimOuNao(mGame->MostrarHitbox())); break;
        }
    }

    if (!mDescricao) return;
    switch (static_cast<Item>(mEscolhida)) {
        case Item::Musica:
            mDescricao->SetText("O volume da m\xc3\xbasica da batalha, de mudo a 10.");
            break;
        case Item::Efeitos:
            mDescricao->SetText("O volume do tiro, do Ponto Extra e dos ataques.");
            break;
        case Item::TelaCheia:
            mDescricao->SetText("O jogo ocupa a tela inteira (no gabinete, sempre).");
            break;
        case Item::Hitbox:
            mDescricao->SetText("Mostra na batalha onde um tiro acerta o aluno.");

            break;
    }
}

void Opcoes::Trocar(const int passo, const bool peloBotao) {
    switch (static_cast<Item>(mEscolhida)) {
        case Item::Musica:
        case Item::Efeitos: {
            const bool musica = static_cast<Item>(mEscolhida) == Item::Musica;
            const int antes = musica ? mGame->VolumeDaMusica() : mGame->VolumeDosEfeitos();
            int depois = Configuracao::MoverVolume(antes, passo);
            if (peloBotao && antes == Configuracao::kVolumeMaximo) depois = 0;
            if (musica) mGame->DefinirVolumeDaMusica(depois);
            else mGame->DefinirVolumeDosEfeitos(depois);
            break;
        }
        case Item::TelaCheia: mGame->DefinirTelaCheia(!mGame->TelaCheia()); break;
        case Item::Hitbox:    mGame->DefinirHitbox(!mGame->MostrarHitbox()); break;
    }
    Mostrar();
}

void Opcoes::OnProcessInput(const Uint8* keyState) {

    const bool voltar = Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Botao::Dois)
                     || keyState[SDL_SCANCODE_ESCAPE];
    if (voltar && !mVoltarAnterior) {
        mGame->RequestSceneChange(SceneType::MainMenu);
        return;
    }
    mVoltarAnterior = voltar;

    // QUALQUER UM DOS DOIS troca, como na pausa: a opcao e da maquina, e nao
    // de um jogador.
    bool esquerda = false, direita = false, cima = false, baixo = false, trocar = false;
    for (const Painel::Jogador quem : Painel::kJogadores) {
        esquerda = esquerda || Painel::Apertado(keyState, quem, Painel::Direcao::Esquerda);
        direita = direita || Painel::Apertado(keyState, quem, Painel::Direcao::Direita);
        cima = cima || Painel::Apertado(keyState, quem, Painel::Direcao::Cima);
        baixo = baixo || Painel::Apertado(keyState, quem, Painel::Direcao::Baixo);
        trocar = trocar || Painel::Apertado(keyState, quem, Painel::Botao::Um);
    }
    trocar = trocar || keyState[SDL_SCANCODE_RETURN] || keyState[SDL_SCANCODE_KP_ENTER];

    // ESCOLHER: para cima e para baixo, dando a volta, como no menu.
    int linha = 0;
    if (cima && !mCimaAnterior) linha -= 1;
    if (baixo && !mBaixoAnterior) linha += 1;
    mCimaAnterior = cima;
    mBaixoAnterior = baixo;
    if (linha != 0) {
        mEscolhida = (mEscolhida + linha + kItens) % kItens;
        Mostrar();
    }

    // TROCAR o valor da linha escolhida.
    const bool peloBotao = trocar && !mTrocarAnterior;
    int passo = 0;
    if (esquerda && !mEsquerdaAnterior) passo -= 1;
    if (direita && !mDireitaAnterior) passo += 1;
    if (peloBotao) passo += 1;
    mEsquerdaAnterior = esquerda;
    mDireitaAnterior = direita;
    mTrocarAnterior = trocar;

    if (passo != 0) Trocar(passo, peloBotao);
}
