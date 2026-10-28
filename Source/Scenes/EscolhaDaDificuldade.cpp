//
// A escolha da dificuldade, antes da selecao de fases.
//

#include "EscolhaDaDificuldade.h"

#include <cstdio>

#include <SDL_scancode.h>

#include "FundoDeCena.h"
#include "../Game.h"
#include "../Font.h"
#include "../Painel.h"
#include "../CaminhosArquivo.h"
#include "../Actors/Actor.h"
#include "../Components/DrawComponents/DrawCaixaComponent.h"
#include "../Components/DrawComponents/DrawTextComponent.h"

namespace {
    constexpr int kLarguraDoCartao = 420;
    constexpr int kAlturaDoCartao = 300;
    constexpr float kEntreCartoes = 250.0f;   ///< do meio da tela ao meio de cada cartao

    // As cores da linha escolhida das Opcoes e do menu: o mesmo dourado.
    const SDL_Color kMolduraEscolhida{255, 214, 90, 255};
    const SDL_Color kFundoEscolhido{58, 52, 34, 210};
    const SDL_Color kMoldura{235, 235, 235, 90};
    const SDL_Color kFundo{34, 38, 48, 170};

    /// "1,2 s" - com virgula, que e texto para quem le, e nao dado.
    std::string Segundos(const float s) {
        char buffer[16];
        std::snprintf(buffer, sizeof(buffer), "%.1f s", static_cast<double>(s));
        std::string t = buffer;
        for (char& c : t) if (c == '.') c = ',';
        return t;
    }
}

EscolhaDaDificuldade::EscolhaDaDificuldade(Game* game)
    : Scene(game, SceneType::EscolhaDaDificuldade),
      mFonte(std::make_unique<Font>())
{
    mFonte->Load(Caminhos::Asset("Fonts/Zelda.ttf"));
}

void EscolhaDaDificuldade::Load() {

    CriarFundoDoCorredor(*this);

    const auto largura = static_cast<float>(mGame->GetWindowWidth());
    const auto altura = static_cast<float>(mGame->GetWindowHeight());
    const float meio = largura / 2.0f;
    const float yDosCartoes = altura * 0.50f;

    Texto("DIFICULDADE", meio, altura * 0.15f, 52, 760);
    Texto(mGame->QuantosJogadores() > 1 ? "Escolham juntos como v\xc3\xa3o encarar o semestre."
                                        : "Como voc\xc3\xaa vai encarar o semestre?",
          meio, altura * 0.24f, 24, 900);

    for (int k = 0; k < Dificuldade::kQuantos; ++k) {
        const Dificuldade::Nivel n = Dificuldade::kNiveis[k];
        const float x = meio + (k == 0 ? -kEntreCartoes : kEntreCartoes);

        auto cartao = std::make_unique<Actor>(this);
        cartao->SetPosition(Vector2(x, yDosCartoes));
        mCartoes[static_cast<size_t>(k)] = cartao->AddComponent<DrawCaixaComponent>(
            kLarguraDoCartao, kAlturaDoCartao, kMoldura, kFundo, 3, 100);
        AddActor(std::move(cartao));

        // O NOME NA COR DO NIVEL: o vermelho do Dificil e o do selo da batalha.
        auto* nome = Texto(Dificuldade::Nome(n), x, yDosCartoes - 95.0f, 44, 380)
                         ->GetComponent<DrawTextComponent>();
        if (n == Dificuldade::Nivel::Dificil) {
            const float* c = Dificuldade::kCorDoDificil;
            nome->SetColor(Vector3(c[0], c[1], c[2]));
        }
        Texto(Dificuldade::Descricao(n), x, yDosCartoes - 5.0f, 22, 360, 3);
        Texto("Depois de um acerto, " + Segundos(Dificuldade::Invencibilidade(n)) + " piscando.",
              x, yDosCartoes + 95.0f, 20, 360);
    }

    Texto(Painel::Rodape({Painel::Manche(Painel::Jogador::Um, "escolher"),
                          {Painel::Jogador::Um, Painel::Botao::Um, "confirmar"},
                          {Painel::Jogador::Um, Painel::Botao::Dois, "trocar aluno"}},
                         mGame->JeitoDoRodape()),
          meio, altura * 0.89f, 24, 900);

    // O CURSOR COMECA ONDE A PARTIDA ESTA: no Normal numa partida nova, e no
    // que foi escolhido quando se volta da selecao de fases.
    for (int k = 0; k < Dificuldade::kQuantos; ++k) {
        if (Dificuldade::kNiveis[k] == mGame->NivelDeDificuldade()) mEscolhido = k;
    }
    Mostrar();
}

Actor* EscolhaDaDificuldade::Texto(const std::string& conteudo, const float x, const float y,
                                   const int tamanho, const int larguraMaxima, const int linhas) {
    auto ator = std::make_unique<Actor>(this);
    ator->SetPosition(Vector2(x, y));

    auto dc = ator->AddComponent<DrawTextComponent>(conteudo, mFonte.get(),
                                                    larguraMaxima, (tamanho + 8) * linhas, tamanho, 255);
    dc->SetLarguraDeQuebra(static_cast<unsigned>(larguraMaxima));
    dc->SetAjustarAoTexto(true);

    Actor* bruto = ator.get();
    AddActor(std::move(ator));
    return bruto;
}

void EscolhaDaDificuldade::Mostrar() {
    for (int k = 0; k < Dificuldade::kQuantos; ++k) {
        DrawCaixaComponent* c = mCartoes[static_cast<size_t>(k)];
        if (!c) continue;
        c->SetMoldura(k == mEscolhido ? kMolduraEscolhida : kMoldura);
        c->SetFundo(k == mEscolhido ? kFundoEscolhido : kFundo);
    }
}

void EscolhaDaDificuldade::OnProcessInput(const Uint8* keyState) {

    const bool voltar = Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Botao::Dois)
                     || keyState[SDL_SCANCODE_ESCAPE];
    if (voltar && !mVoltarAnterior) {
        mGame->RequestSceneChange(SceneType::Identificacao);
        return;
    }
    mVoltarAnterior = voltar;

    // QUALQUER UM DOS DOIS escolhe: os dois jogam no mesmo nivel. Para cima e
    // para baixo tambem trocam - com dois cartoes, todo lado e o outro.
    bool esquerda = false, direita = false, confirmar = false;
    for (const Painel::Jogador quem : Painel::kJogadores) {
        esquerda = esquerda || Painel::Apertado(keyState, quem, Painel::Direcao::Esquerda)
                            || Painel::Apertado(keyState, quem, Painel::Direcao::Cima);
        direita = direita || Painel::Apertado(keyState, quem, Painel::Direcao::Direita)
                          || Painel::Apertado(keyState, quem, Painel::Direcao::Baixo);
        confirmar = confirmar || Painel::Apertado(keyState, quem, Painel::Botao::Um);
    }
    confirmar = confirmar || keyState[SDL_SCANCODE_RETURN] || keyState[SDL_SCANCODE_KP_ENTER];

    int passo = 0;
    if (esquerda && !mEsquerdaAnterior) passo -= 1;
    if (direita && !mDireitaAnterior) passo += 1;
    mEsquerdaAnterior = esquerda;
    mDireitaAnterior = direita;
    if (passo != 0) {
        mEscolhido = (mEscolhido + passo + Dificuldade::kQuantos) % Dificuldade::kQuantos;
        Mostrar();
    }

    if (confirmar && !mConfirmarAnterior) {
        mConfirmarAnterior = true;
        mGame->DefinirDificuldade(Dificuldade::kNiveis[mEscolhido]);
        mGame->RequestSceneChange(SceneType::StageSelect);
        return;
    }
    mConfirmarAnterior = confirmar;
}

void EscolhaDaDificuldade::OnUpdate(const float deltaTime) {

}
