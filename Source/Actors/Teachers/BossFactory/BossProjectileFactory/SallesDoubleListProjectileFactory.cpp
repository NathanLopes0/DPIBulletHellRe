//
// Created by gensh on 09/04/2026.
//

#include "SallesDoubleListProjectileFactory.h"

#include <vector>
#include <complex>

#include "../../Bosses/Salles.h"
#include "../../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../../../Components/RigidBodyComponent.h"
#include "../../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Bosses/BossesProjectiles/SallesDoubleListProjectile.h"
#include <SDL_log.h>

std::unique_ptr<Projectile> SallesDoubleListProjectileFactory::createProjectile(Scene* scene, Actor* owner) {

    auto BossOwner = dynamic_cast<Salles*>(owner);
    if (!BossOwner) {
        SDL_Log("ERRO FATAL: SallesDoubleListProjectileFactory recebeu um owner que não é um Salles");
        return nullptr;
    }



    // -- CRIAÇÃO DO PROJÉTIL -- //
    //SDL_Log("Criando Projetil DoubleList");
    auto projectile = std::make_unique<SallesDoubleListProjectile>(scene,BossOwner);
    projectile->SetScale(.75f);

    projectile->AddComponent<RigidBodyComponent>();

    //SDL_Log("Criando drawComponent");
    auto drawComp = projectile->AddComponent<DrawAnimatedComponent>(mSpritePath, mDataPath, 90);


    drawComp->AddAnimation("Normal", {0,1,2,3,4,5,6,7,8});
    drawComp->SetAnimation("Normal");

    drawComp->SetIsVisible(true);

    float colliderRadius = static_cast<float>(drawComp->GetSpriteHeight()) / 2.f;
    projectile->AddComponent<CircleColliderComponent>(colliderRadius);

    projectile->SetState(ActorState::Active);

    //SDL_Log("Retornando o projetil");
    return projectile;



}

std::unique_ptr<Projectile> SallesDoubleListProjectileFactory::Acquire(Scene* scene, Actor* owner) {

    auto created = mPool.Acquire([this, scene, owner]() {
        return std::unique_ptr<SallesDoubleListProjectile>(
            dynamic_cast<SallesDoubleListProjectile*>(createProjectile(scene, owner).release())
        );
    });

    if (!created) {
        SDL_Log("ERRO FATAL: SallesDoubleListProjectileFactory::Acquire falhou em criar/reciclar projetil!");
        return nullptr;
    }

    created->SetOwner(owner);

    // Posicao base igual a de um projetil recem-construido: createProjectile
    // deixa o objeto na posicao do dono, mas quem vem do pool nao passa por
    // ela e carregaria a posicao onde morreu. Sem isto, "reciclado" e "novo"
    // se comportam diferente, que e a raiz do bug de posicao. A strategy
    // continua livre para sobrescrever logo em seguida.
    if (owner) {
        created->SetPosition(owner->GetPosition());
    }
    created->SetOriginFactory(this);

    return created;
}

void SallesDoubleListProjectileFactory::Release(std::unique_ptr<Projectile> projectile) {

    if (!dynamic_cast<SallesDoubleListProjectile*>(projectile.get())) {
        SDL_Log("ERRO: SallesDoubleListProjectileFactory::Release recebeu um projetil de tipo incompativel!");
        return;
    }

    mPool.Release(std::unique_ptr<SallesDoubleListProjectile>(
        dynamic_cast<SallesDoubleListProjectile*>(projectile.release())
    ));
}

void SallesDoubleListProjectileFactory::Prewarm(Scene* scene, Actor* owner, int count) {

    std::vector<std::unique_ptr<Projectile>> held;
    held.reserve(count);

    for (int i = 0; i < count; ++i) {
        auto p = Acquire(scene, owner);
        if (!p) {
            SDL_Log("AVISO: SallesDoubleListProjectileFactory::Prewarm falhou ao criar instancia %d de %d.", i, count);
            continue;
        }
        held.push_back(std::move(p));
    }

    for (auto& p : held) {
        Release(std::move(p));
    }
}
