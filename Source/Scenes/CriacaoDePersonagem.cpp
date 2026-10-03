//
// A tela onde o aluno escolhe a personagem dele.
//

#include "CriacaoDePersonagem.h"

#include <SDL_scancode.h>

#include "../Game.h"
#include "../Font.h"
#include "../CaminhosArquivo.h"
#include "../ComporPersonagem.h"
#include "../PersonagensArquivo.h"
#include "../Actors/Actor.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Components/DrawComponents/DrawTextComponent.h"

namespace {

    /// A personagem e desenhada em quadros de 64; sem ampliar, ela sairia do
    /// tamanho que tem em jogo - pequena demais para escolher olhando.
    constexpr float kAmpliacao = 4.0f;

    constexpr float kAlturaDaPersonagem = 0.42f;
}

CriacaoDePersonagem::CriacaoDePersonagem(Game* game)
    : Scene(game, SceneType::CriacaoDePersonagem),
      mFonte(std::make_unique<Font>())
{
    mFonte->Load(Caminhos::Asset("Fonts/Zelda.ttf"));
}

Actor* CriacaoDePersonagem::Texto(const std::string& conteudo, const float y,
                                  const int tamanho, const int larguraMaxima) {

    auto ator = std::make_unique<Actor>(this);
    ator->SetPosition(Vector2(static_cast<float>(mGame->GetWindowWidth()) / 2.0f, y));

    auto dc = ator->AddComponent<DrawTextComponent>(conteudo, mFonte.get(),
                                                    larguraMaxima, tamanho + 8, tamanho, 255);
    dc->SetLarguraDeQuebra(static_cast<unsigned>(larguraMaxima));
    dc->SetAjustarAoTexto(true);

    Actor* bruto = ator.get();
    AddActor(std::move(ator));
    return bruto;
}

void CriacaoDePersonagem::Load() {

    const auto largura = static_cast<float>(mGame->GetWindowWidth());
    const auto altura  = static_cast<float>(mGame->GetWindowHeight());

    Texto("SUA PERSONAGEM", altura * 0.11f, 52, 900);

    const Personagens::Catalogo& catalogo = Personagens::Carregado();

    for (const auto& pronta : catalogo.predefinidas) {

        const Personagens::Composta composta =
            Personagens::Compor(mGame, catalogo, pronta.aparencia);

        if (!composta.ok) {
            // Ja relatado por quem tentou compor. Pular esta e seguir e melhor
            // do que uma tela vazia: as outras combinacoes continuam servindo.
            SDL_Log("CRIACAO: a combinacao \"%s\" nao pode ser composta; nao sera oferecida.",
                    pronta.id.c_str());
            continue;
        }

        auto ator = std::make_unique<Actor>(this);
        ator->SetPosition(Vector2(largura / 2.0f, altura * kAlturaDaPersonagem));
        ator->SetScale(kAmpliacao);

        auto dc = ator->AddComponent<DrawAnimatedComponent>(composta.chaveDaTextura,
                                                            composta.atlas);
        dc->AddAnimation("Andando", {0, 1, 2, 3});
        dc->SetAnimation("Andando");
        dc->SetAnimFPS(6.0f);   // mais devagar que em jogo: aqui e para olhar
        dc->SetIsVisible(false);

        mPersonagens.push_back(ator.get());
        AddActor(std::move(ator));
    }

    if (mPersonagens.empty()) {
        // Sem nenhuma personagem nao ha o que escolher. Dizer isso e melhor do
        // que uma tela preta: ENTER segue com a aparencia padrao.
        Texto("Nenhuma personagem disponivel.", altura * kAlturaDaPersonagem, 28, 900);
        Texto("Confira Assets/personagens.json.", altura * 0.52f, 24, 900);
    }

    mNomeAtor     = Texto(" ", altura * 0.64f, 34, 900);
    mContadorAtor = Texto(" ", altura * 0.71f, 24, 400);

    Texto("SETAS  escolher",   altura * 0.81f, 24, 520);
    Texto("ENTER  confirmar",  altura * 0.87f, 24, 520);
    Texto("ESC  voltar",       altura * 0.93f, 24, 520);

    Mostrar();
}

void CriacaoDePersonagem::Mostrar() const {

    for (size_t i = 0; i < mPersonagens.size(); ++i) {
        if (auto dc = mPersonagens[i]->GetComponent<DrawAnimatedComponent>()) {
            dc->SetIsVisible(static_cast<int>(i) == mEscolhida);
        }
    }

    const auto& prontas = Personagens::Carregado().predefinidas;

    if (mNomeAtor) {
        if (auto dc = mNomeAtor->GetComponent<DrawTextComponent>()) {
            const bool temNome = mEscolhida >= 0
                              && mEscolhida < static_cast<int>(prontas.size());
            dc->SetText(temNome ? prontas[static_cast<size_t>(mEscolhida)].nome : " ");
        }
    }

    if (mContadorAtor) {
        if (auto dc = mContadorAtor->GetComponent<DrawTextComponent>()) {
            // Saber quantas faltam evita o aluno ficar batendo na seta sem saber
            // se ja viu todas.
            dc->SetText(mPersonagens.empty()
                            ? " "
                            : std::to_string(mEscolhida + 1) + " de " +
                              std::to_string(mPersonagens.size()));
        }
    }
}

void CriacaoDePersonagem::OnProcessInput(const Uint8* keyState) {

    const int quantas = static_cast<int>(mPersonagens.size());

    const bool esquerda = keyState[SDL_SCANCODE_LEFT]  || keyState[SDL_SCANCODE_A];
    const bool direita  = keyState[SDL_SCANCODE_RIGHT] || keyState[SDL_SCANCODE_D];

    if (quantas > 0) {
        if (esquerda && !mEsquerdaAnterior) {
            // Soma quantas antes do resto: em C++ o resto de um negativo e
            // negativo, e -1 % n nao daria a volta para a ultima.
            mEscolhida = (mEscolhida - 1 + quantas) % quantas;
            Mostrar();
        }
        if (direita && !mDireitaAnterior) {
            mEscolhida = (mEscolhida + 1) % quantas;
            Mostrar();
        }
    }
    mEsquerdaAnterior = esquerda;
    mDireitaAnterior = direita;

    const bool confirmar = keyState[SDL_SCANCODE_RETURN] || keyState[SDL_SCANCODE_KP_ENTER];
    if (confirmar && !mConfirmarAnterior) {
        mConfirmarAnterior = true;

        const auto& prontas = Personagens::Carregado().predefinidas;
        if (mEscolhida >= 0 && mEscolhida < static_cast<int>(prontas.size())) {
            // Grava na hora: quem acabou de montar a personagem espera
            // encontra-la ao carregar o perfil, mesmo fechando o jogo agora.
            mGame->DefinirAparencia(prontas[static_cast<size_t>(mEscolhida)].aparencia);
        }

        mGame->RequestSceneChange(SceneType::StageSelect);
        return;
    }
    mConfirmarAnterior = confirmar;

    const bool voltar = keyState[SDL_SCANCODE_ESCAPE];
    if (voltar && !mVoltarAnterior) {
        // Volta para a matricula, e nao para o menu: quem chegou aqui ja digitou
        // uma matricula e o perfil dela ja existe em disco.
        mGame->RequestSceneChange(SceneType::Identificacao);
        return;
    }
    mVoltarAnterior = voltar;
}

void CriacaoDePersonagem::OnUpdate(const float deltaTime) {

}
