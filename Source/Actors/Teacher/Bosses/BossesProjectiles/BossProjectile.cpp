//
// Created by nslop on 21/11/2024.
//
// BossProjectile.cpp (VERSÃO REFATORADA E MODERNA)

#include "BossProjectile.h"
#include "../../Boss.h"
#include "../../../../Components/DrawComponents/DrawAnimatedComponent.h" // Para GetComponent
#include "../../../../Scenes/Scene.h"  // Para GetGame()
#include "../../../../Game.h"         // Para GetWindowWidth/Height

// 1. O construtor agora passa o 'owner' para o construtor da classe base 'Projectile'.
BossProjectile::BossProjectile(Scene* scene, Boss* owner)
    : Projectile(scene, owner) // Chama o construtor do pai com a cena E o dono
{
    // A responsabilidade de adicionar componentes (Draw, Collider)
    // será da ProjectileFactory que criar este projétil.
    // O construtor do projétil em si fica limpo!
}

// 2. OnUpdate se torna o motor para os nossos futuros Behaviors.
void BossProjectile::OnUpdate(float deltaTime)
{
    // Primeiro, executa a lógica base do Projectile (verificar se está fora da tela).
    Projectile::OnUpdate(deltaTime);

    // TODO: Implementar o loop de Behaviors quando tivermos a classe base.
    // Exemplo de como será:
    // for (const auto& behavior : mBehaviors)
    // {
    //     behavior->Update(this, deltaTime);
    // }
}

// 3. Implementação obrigatória de IsOffScreen, com a lógica corrigida.
bool BossProjectile::IsOffScreen() const
{
    // Acessamos os componentes de forma segura usando GetComponent.
    if (auto drawComp = GetComponent<DrawAnimatedComponent>())
    {
        const auto pos = GetPosition();
        const auto game = mOwner->GetScene()->GetGame();
        const auto spriteWidth = static_cast<float>(drawComp->GetSpriteWidth());
        const auto spriteHeight = static_cast<float>(drawComp->GetSpriteHeight());

        // A lógica correta: retorna 'true' se estiver FORA dos limites.
        return pos.x < -spriteWidth ||
               pos.x > static_cast<float>(game->GetWindowWidth()) + spriteWidth ||
               pos.y < -spriteHeight ||
               pos.y > static_cast<float>(game->GetWindowHeight()) + spriteHeight;
    }

    // Se não tiver componente de desenho, consideramos que não pode sair da tela.
    return false;
}

// 4. Implementação do novo getter seguro.
Boss* BossProjectile::GetBossOwner() const
{
    // Usamos dynamic_cast para converter com segurança o Actor* da classe base
    // para o Boss* que sabemos que ele é. Retorna nullptr se o cast falhar.
    return dynamic_cast<Boss*>(mOwner);
}