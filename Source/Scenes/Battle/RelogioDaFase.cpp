//
// O relogio da fase.
//

#include "RelogioDaFase.h"

#include <cmath>
#include <memory>

#include "../Scene.h"
#include "../../Actors/Actor.h"
#include "../../Actors/Teachers/Boss.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Components/DrawComponents/DrawRelogioDaFaseComponent.h"
#include "../../Interface/Hud.h"
#include "../../Motor/CaminhosArquivo.h"

RelogioDaFase::RelogioDaFase(Scene& cena, const Boss& chefe) {
    const int altura = chefe.GetComponent<DrawAnimatedComponent>()->GetSpriteHeight();

    auto capsula = std::make_unique<Actor>(&cena);
    mDesenhoDaCapsula = capsula->AddComponent<DrawRelogioDaFaseComponent>(
        12, static_cast<int>(static_cast<float>(altura) / 1.4f), 200);
    mCapsula = capsula.get();
    cena.AddActor(std::move(capsula));

    // O reloginho embaixo da capsula.
    auto relogio = std::make_unique<Actor>(&cena);
    auto* desenho = relogio->AddComponent<DrawAnimatedComponent>(Caminhos::Asset("Icons/TestTimeIcon.png"),
                                                                 Caminhos::Asset("Icons/TestTimeIcon.json"), 201);
    desenho->AddAnimation("Idle", {0});
    desenho->AddAnimation("Active", {0, 1, 2, 3, 4, 5});
    desenho->SetAnimation("Active");
    mRelogio = relogio.get();
    cena.AddActor(std::move(relogio));
}

void RelogioDaFase::Seguir(const Boss& chefe, const float restante) {
    const Vector2 posicao = chefe.GetPosition();
    const auto* desenho = chefe.GetComponent<DrawAnimatedComponent>();
    const auto largura = desenho->GetSpriteWidth();
    const auto altura = desenho->GetSpriteHeight();

    const float dx = static_cast<float>(largura) / 2.5f;
    const float dy = static_cast<float>(altura) / 4.f;
    mCapsula->SetPosition(Vector2(posicao.x + dx, posicao.y - dy));
    mRelogio->SetPosition(mCapsula->GetPosition() + Vector2(6, static_cast<float>(altura) / 1.4f + 4.0f));

    // O relogio fica vermelho junto com a capsula, nos ultimos segundos.
    if (auto* relogio = mRelogio->GetComponent<DrawAnimatedComponent>()) {
        const bool urgente = Hud::Urgente(restante);
        const bool aceso = urgente && std::fmod(restante, 0.5f) > 0.25f;
        if (aceso) relogio->SetColor(255, 110, 100);
        else relogio->SetColor(255, 255, 255);
    }
}
