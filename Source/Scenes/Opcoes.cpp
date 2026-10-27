//
// A tela de opcoes.
//

#include "Opcoes.h"
#include "../Painel.h"
#include "FundoDeCena.h"

#include <SDL_scancode.h>

#include "../Game.h"
#include "../Font.h"
#include "../Dificuldade.h"
#include "../CaminhosArquivo.h"
#include "../Actors/Actor.h"
#include "../Components/DrawComponents/DrawCaixaComponent.h"
#include "../Components/DrawComponents/DrawTextComponent.h"

namespace {
    // A LINHA DA OPCAO: uma moldura com o nome a esquerda e o valor a direita,
    // como as linhas do menu principal.
    constexpr int kLarguraDaLinha = 640;
    constexpr int kAlturaDaLinha = 64;
    constexpr float kRecuoDoTexto = 150.0f;   ///< do meio da linha ao meio de cada texto
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

    // \xc3\x87 e \xc3\x95 sao o C cedilha e o O til: o MSVC nao le acento em literal.
    Texto("OP\xc3\x87\xc3\x95" "ES", meio, altura * 0.22f, 52, 760);

    auto linha = std::make_unique<Actor>(this);
    linha->SetPosition(Vector2(meio, altura * 0.45f));
    linha->AddComponent<DrawCaixaComponent>(kLarguraDaLinha, kAlturaDaLinha,
                                            SDL_Color{235, 235, 235, 255}, SDL_Color{34, 38, 48, 150},
                                            3, 100);
    AddActor(std::move(linha));

    Texto("Dificuldade", meio - kRecuoDoTexto, altura * 0.45f, 34, 300);
    mValor = Texto("", meio + kRecuoDoTexto, altura * 0.45f, 34, 300)->GetComponent<DrawTextComponent>();
    mDescricao = Texto("", meio, altura * 0.56f, 24, 760)->GetComponent<DrawTextComponent>();
    MostrarDificuldade();

    // O BOTAO 1 TAMBEM TROCA, sem aparecer escrito - como o ENTER: duas
    // entradas com o mesmo "trocar" no rodape pareciam duas acoes diferentes.
    Texto(Painel::Rodape({Painel::Manche(Painel::Jogador::Um, "trocar"),
                          {Painel::Jogador::Um, Painel::Botao::Dois, "voltar"}},
                         mGame->JeitoDoRodape()),
          meio, altura * 0.80f, 24, 760);
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

void Opcoes::MostrarDificuldade() {
    const Dificuldade::Nivel n = mGame->NivelDeDificuldade();
    if (mValor) {
        mValor->SetText(std::string("< ") + Dificuldade::Nome(n) + " >");
        const float* c = Dificuldade::kCorDoDificil;
        mValor->SetColor(n == Dificuldade::Nivel::Dificil ? Vector3(c[0], c[1], c[2]) : Vector3(1.0f, 1.0f, 1.0f));
    }
    if (mDescricao) mDescricao->SetText(Dificuldade::Descricao(n));
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
    bool esquerda = false, direita = false, trocar = false;
    for (const Painel::Jogador quem : Painel::kJogadores) {
        esquerda = esquerda || Painel::Apertado(keyState, quem, Painel::Direcao::Esquerda);
        direita = direita || Painel::Apertado(keyState, quem, Painel::Direcao::Direita);
        trocar = trocar || Painel::Apertado(keyState, quem, Painel::Botao::Um);
    }
    trocar = trocar || keyState[SDL_SCANCODE_RETURN] || keyState[SDL_SCANCODE_KP_ENTER];

    int passo = 0;
    if (esquerda && !mEsquerdaAnterior) passo -= 1;
    if (direita && !mDireitaAnterior) passo += 1;
    if (trocar && !mTrocarAnterior) passo += 1;
    mEsquerdaAnterior = esquerda;
    mDireitaAnterior = direita;
    mTrocarAnterior = trocar;

    if (passo != 0) {
        mGame->DefinirDificuldade(Dificuldade::Mover(mGame->NivelDeDificuldade(), passo));
        MostrarDificuldade();
    }
}

void Opcoes::OnUpdate(const float deltaTime) {

}
