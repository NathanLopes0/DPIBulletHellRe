//
// O fundo que as telas de menu compartilham.
//

#include "FundoDeCena.h"

#include <memory>

#include "Scene.h"
#include "../CaminhosArquivo.h"
#include "../Game.h"
#include "../Actors/Actor.h"
#include "../Components/DrawComponents/DrawCaixaComponent.h"
#include "../Components/DrawComponents/DrawSpriteComponent.h"

namespace {

    /// A ordem de desenho e crescente, e o padrao dos componentes e 100. Estes
    /// dois ficam embaixo de tudo, e o veu em cima da foto.
    constexpr int kOrdemDaFoto = 40;
    constexpr int kOrdemDoVeu = 45;

    /// Quanto a foto e escurecida ANTES do veu. Puxada para o azul: o corredor
    /// e de tijolo vermelho, e um cinza neutro deixava o fundo amarelado.
    constexpr Uint8 kTinturaR = 66;
    constexpr Uint8 kTinturaG = 70;
    constexpr Uint8 kTinturaB = 92;

    /// O veu por cima. Sem ele, o nome de uma materia fechada cai em cima de um
    /// reflexo de janela e some.
    constexpr SDL_Color kSemBorda = {0, 0, 0, 0};
    constexpr SDL_Color kVeu = {6, 8, 18, 200};
}

void CriarFundoDoCorredor(Scene& cena) {

    Game* jogo = cena.GetGame();
    if (!jogo) return;

    const auto largura = static_cast<float>(jogo->GetWindowWidth());
    const auto altura = static_cast<float>(jogo->GetWindowHeight());
    const Vector2 meio(largura / 2.0f, altura / 2.0f);

    auto foto = std::make_unique<Actor>(&cena);
    foto->SetPosition(meio);
    const auto sprite = foto->AddComponent<DrawSpriteComponent>(
        Caminhos::Asset("MainMenuBackground.png"), kOrdemDaFoto);
    sprite->SetColor(kTinturaR, kTinturaG, kTinturaB);
    cena.AddActor(std::move(foto));

    auto veu = std::make_unique<Actor>(&cena);
    veu->SetPosition(meio);
    veu->AddComponent<DrawCaixaComponent>(
        static_cast<int>(largura), static_cast<int>(altura),
        kSemBorda, kVeu, 0, kOrdemDoVeu);
    cena.AddActor(std::move(veu));
}
