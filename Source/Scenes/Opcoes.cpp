//
// A tela de opcoes. Vazia por enquanto, de proposito.
//

#include "Opcoes.h"

#include <SDL_scancode.h>

#include "../Game.h"
#include "../Font.h"
#include "../CaminhosArquivo.h"
#include "../Actors/Actor.h"
#include "../Components/DrawComponents/DrawTextComponent.h"

Opcoes::Opcoes(Game* game)
    : Scene(game, SceneType::Opcoes),
      mFonte(std::make_unique<Font>())
{
    mFonte->Load(Caminhos::Asset("Fonts/Zelda.ttf"));
}

void Opcoes::Load() {

    const auto altura = static_cast<float>(mGame->GetWindowHeight());

    Texto("OPCOES", altura * 0.22f, 52, 760);
    Texto("Nada para configurar ainda.", altura * 0.45f, 28, 760);
    Texto("ESC  voltar", altura * 0.80f, 24, 420);
}

Actor* Opcoes::Texto(const std::string& conteudo, const float y, const int tamanho,
                     const int larguraMaxima) {

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

void Opcoes::OnProcessInput(const Uint8* keyState) {

    const bool voltar = keyState[SDL_SCANCODE_ESCAPE];
    if (voltar && !mVoltarAnterior) {
        mGame->RequestSceneChange(SceneType::MainMenu);
        return;
    }
    mVoltarAnterior = voltar;
}

void Opcoes::OnUpdate(const float deltaTime) {

}
