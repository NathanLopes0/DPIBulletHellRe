//
// Created by nslop on 21/11/2024.
//
// BossProjectile.cpp (VERSÃO REFATORADA E MODERNA)

#include "BossProjectile.h"
#include "../../Boss.h"
#include "../../../../Game.h"         // Para GetWindowWidth/Height
#include "../../../../Components/DrawComponents/DrawAnimatedComponent.h" // Para GetComponent<>()
#include "../../../../Scenes/Battle/Battle.h"
#include "../../../Player/Player.h"


// 1. O construtor passa o 'owner' para o construtor da classe base 'Projectile'.
BossProjectile::BossProjectile(Scene* scene, Boss* owner)
    : Projectile(scene, owner) // Chama o construtor do pai com a cena E o dono
{

}



void BossProjectile::OnUpdate(float deltaTime)
{
    // 1. Executa a lógica base (verificar se saiu da tela, update nos Behaviors).
    Projectile::OnUpdate(deltaTime);
}

// 3. Implementação obrigatória de IsOffScreen, com a lógica corrigida.
bool BossProjectile::IsOffScreen() const
{
    // Acessamos os componentes de forma segura
    if (auto drawComp = GetComponent<DrawAnimatedComponent>())
    {
        // 4. Acessamos a Scene e o Game diretamente da classe Actor base!
        const auto pos = GetPosition();
        const auto game = mScene->GetGame();
        const auto windowWidth = static_cast<float>(game->GetWindowWidth());
        const auto windowHeight = static_cast<float>(game->GetWindowHeight());
        const auto spriteWidth = static_cast<float>(drawComp->GetSpriteWidth());
        const auto spriteHeight = static_cast<float>(drawComp->GetSpriteHeight());

        // "Zona de buffer"
        const float hBuffer = windowWidth / 12.0f;
        const float vBuffer = windowHeight / 12.0f;

        // Retorna 'true' se estiver FORA dos limites.
        return pos.x < -spriteWidth - hBuffer ||
               pos.x > windowWidth + spriteWidth + hBuffer ||
               pos.y < -spriteHeight - vBuffer ||
               pos.y > windowHeight + spriteHeight + vBuffer;
    }

    // Failsafe se não houver componente de desenho
    SDL_Log("Erro em BossProjectile.cpp - IsOffScreen: projetil nao tem DrawAnimatedComponent, retornando false");
    return false;
}

// 4. Implementação do novo getter seguro.
Boss* BossProjectile::GetBossOwner() const
{
    // Usamos dynamic_cast para converter com segurança o Actor* da classe base
    // para o Boss* que sabemos que ele é. Retorna nullptr se o cast falhar.
    return dynamic_cast<Boss*>(mOwner);
}

Vector2 BossProjectile::GetPlayerPosition() const {
    if (auto battleScene = dynamic_cast<Battle*>(mScene)) {
        if (auto player = battleScene->GetPlayer()) {
            return player->GetPosition();
        }
        SDL_Log("Erro em BossProjectile.cpp - GetPlayerPosition: nao foi possivel encontrar o Player"
                "Retornando vetor base");
        return {};
    }

    SDL_Log("Erro em BossProjectile.cpp - GetPlayerPosition: tentou encontrar cena Battle e nao achou."
            "Retornando vetor base");
    return {};
}

Vector2 BossProjectile::GetPlayerDirection() const {
    const Vector2 playerPos = GetPlayerPosition();
    Vector2 direction = playerPos - GetPosition();
    direction.Normalize();

    return direction;
}
