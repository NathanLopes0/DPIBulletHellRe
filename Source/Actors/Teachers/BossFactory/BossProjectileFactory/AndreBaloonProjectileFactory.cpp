//
// Created by gensh on 24/03/2026.
//

#include "AndreBaloonProjectileFactory.h"

#include <vector>
#include <SDL_log.h>
#include "../../Bosses/Andre.h"
#include "../../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../../../Components/RigidBodyComponent.h"
#include "../../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Bosses/BossesProjectiles/AndreBaloonProjectile.h"

std::unique_ptr<Projectile> AndreBaloonProjectileFactory::createProjectile(Scene *scene, Actor *owner) {

    auto bossOwner = dynamic_cast<Andre*>(owner);
    if (!bossOwner) {
        SDL_Log("ERRO FATAL: AndreBaloonProjectileFactory recebeu um 'owner' que nao e um Boss!");
        return nullptr;
    }

    // -- CRIAÇÃO DO PROJÉTIL -- //

    auto projectile = std::make_unique<AndreBaloonProjectile>(scene, bossOwner);

    projectile->AddComponent<RigidBodyComponent>();

    projectile->SetScale(2.f);

    auto drawComp = projectile->AddComponent<DrawAnimatedComponent>("../Assets/Teachers/Projectiles/DPIBHAndreBaloon.png",
                                                "../Assets/Teachers/Projectiles/DPIBHAndreBaloon.json", 90);

    drawComp->AddAnimation("Red", {6, 7, 8, 9, 10, 11});
    drawComp->AddAnimation("Blue", {12,13,14,15,16,17});
    drawComp->AddAnimation("Yellow", {18,19,20,21,22,23});

    drawComp->SetIsVisible(true);

    float colliderRadius = static_cast<float>(drawComp->GetSpriteWidth()) / 4.f;
    projectile->AddComponent<CircleColliderComponent>(colliderRadius);

    projectile->SetState(ActorState::Active);

    return projectile;

}

std::unique_ptr<Projectile> AndreBaloonProjectileFactory::Acquire(Scene* scene, Actor* owner) {

    // A "receita" de criação é a mesma createProjectile de sempre — só é
    // chamada quando o pool está vazio (primeira vez, ou todos os objetos
    // deste tipo já estão em uso simultâneo na tela).
    auto created = mPool.Acquire([this, scene, owner]() {
        return std::unique_ptr<AndreBaloonProjectile>(
            dynamic_cast<AndreBaloonProjectile*>(createProjectile(scene, owner).release())
        );
    });

    if (!created) {
        SDL_Log("ERRO FATAL: AndreBaloonProjectileFactory::Acquire falhou em criar/reciclar projetil!");
        return nullptr;
    }

    // Reatribui owner (pode ser diferente do owner da vez anterior, mesmo
    // reciclando o mesmo objeto físico) e marca de onde este objeto veio,
    // para o ProjectileManager saber para quem devolver quando ele morrer.
    created->SetOwner(owner);
    created->SetOriginFactory(this);

    return created;
}

void AndreBaloonProjectileFactory::Release(std::unique_ptr<Projectile> projectile) {

    auto* raw = dynamic_cast<AndreBaloonProjectile*>(projectile.get());
    if (!raw) {
        SDL_Log("ERRO: AndreBaloonProjectileFactory::Release recebeu um projetil de tipo incompativel!");
        return;
    }

    // Solta a posse de 'projectile' para reembrulhar como unique_ptr<AndreBaloonProjectile>,
    // sem destruir o objeto (mesmo padrão de cast já usado em Boss::ExecuteAttack).
    mPool.Release(std::unique_ptr<AndreBaloonProjectile>(
        dynamic_cast<AndreBaloonProjectile*>(projectile.release())
    ));
}

void AndreBaloonProjectileFactory::Prewarm(Scene* scene, Actor* owner, int count) {

    // Mantemos todas as instâncias vivas AQUI, fora do pool, durante a
    // criação. Se chamássemos Acquire()+Release() em sequência dentro do
    // loop, cada Release() devolveria o MESMO objeto ao pool, e o próximo
    // Acquire() simplesmente o reciclaria — nunca criaríamos mais que 1
    // instância de verdade. Só ao final, quando todas já existem
    // simultaneamente, é que devolvemos todas de uma vez.
    std::vector<std::unique_ptr<Projectile>> held;
    held.reserve(count);

    for (int i = 0; i < count; ++i) {
        auto p = Acquire(scene, owner);
        if (!p) {
            SDL_Log("AVISO: AndreBaloonProjectileFactory::Prewarm falhou ao criar instancia %d de %d.", i, count);
            continue;
        }
        held.push_back(std::move(p));
    }

    for (auto& p : held) {
        Release(std::move(p));
    }
}
