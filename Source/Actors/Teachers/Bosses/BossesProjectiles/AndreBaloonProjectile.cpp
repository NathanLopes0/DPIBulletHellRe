//
// Created by gensh on 24/03/2026.
//

#include "AndreBaloonProjectile.h"

#include "../../../../Game.h"
#include "../../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../../Scenes/Scene.h"
#include "../Andre.h"

AndreBaloonProjectile::AndreBaloonProjectile(Scene *scene, Boss *owner)
    :BossProjectile(scene, owner)
{

}

void AndreBaloonProjectile::OnUpdate(float deltaTime) {
    BossProjectile::OnUpdate(deltaTime);
}

bool AndreBaloonProjectile::IsOffScreen() const {

    // Failsafe: garantir que o componente de desenho existe
    auto drawComp = GetComponent<DrawAnimatedComponent>();
    if (!drawComp) return false;

    const auto spriteHeight = static_cast<float>(drawComp->GetSpriteHeight());
    const auto spriteWidth = static_cast<float>(drawComp->GetSpriteWidth());

    // Pegando as dimensões da tela
    const auto game = mScene->GetGame();
    const auto windowWidth = static_cast<float>(game->GetWindowWidth());
    const auto windowHeight = static_cast<float>(game->GetWindowHeight());

    const auto pos = GetPosition();

    // Verificação de limites verticais
    if (pos.y > windowHeight + 2.0f * spriteHeight || pos.y < -(2.0f * spriteHeight)) {
        return true;
    }

    // Verificação de limites horizontais
    if (pos.x > windowWidth + 2.0f * spriteWidth || pos.x < -(2.0f * spriteWidth)) {
        return true;
    }

    return false;

}
