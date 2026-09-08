//
// Fábrica de projéteis do Júlio (INF 420)
//

#include "JulioProjectile1Factory.h"

#include <SDL_log.h>

#include "../../../../Components/RigidBodyComponent.h"
#include "../../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../Bosses/Julio.h"

std::unique_ptr<Projectile> JulioProjectile1Factory::createProjectile(Scene* scene, Actor* owner) {

    auto* bossOwner = dynamic_cast<Julio*>(owner);
    if (!bossOwner) {
        SDL_Log("ERRO FATAL: JulioProjectile1Factory recebeu um 'owner' que nao e o Julio!");
        return nullptr;
    }

    // -- CRIAÇÃO DO PROJÉTIL -- //
    auto projectile = std::make_unique<JulioBossProjectile>(scene, bossOwner);

    projectile->SetPosition(bossOwner->GetPosition());

    projectile->AddComponent<RigidBodyComponent>();

    // Sprite de 16x16 desenhada em 32x32, igual aos projéteis do Ricardo.
    projectile->SetScale(2.f);

    auto drawComp = projectile->AddComponent<DrawAnimatedComponent>(mSpritePath, mDataPath, 90);

    // Uma animação por fase da luta. Julio::OnProjectilesCreated escolhe qual
    // usar conforme o estado da FSM que disparou o ataque.
    drawComp->AddAnimation("Ruido",     {0, 1, 2, 3});
    drawComp->AddAnimation("Gradiente", {4, 5, 6, 7});
    drawComp->AddAnimation("Overfit",   {8, 9, 10, 11});

    // SEMPRE deixe uma animação válida definida aqui. Um projétil pode ir
    // direto para o pool pelo Prewarm e ser desenhado antes de qualquer ataque
    // escolher a dele.
    drawComp->SetAnimation("Ruido");
    drawComp->SetIsVisible(true);

    const float colliderRadius = static_cast<float>(drawComp->GetSpriteWidth()) / 2.f;
    projectile->AddComponent<CircleColliderComponent>(colliderRadius);

    projectile->SetState(ActorState::Active);

    return projectile;
}

std::unique_ptr<Projectile> JulioProjectile1Factory::Acquire(Scene* scene, Actor* owner) {

    auto created = mPool.Acquire([this, scene, owner]() {
        return std::unique_ptr<JulioBossProjectile>(
            dynamic_cast<JulioBossProjectile*>(createProjectile(scene, owner).release())
        );
    });

    if (!created) {
        SDL_Log("ERRO FATAL: JulioProjectile1Factory::Acquire falhou em criar/reciclar projetil!");
        return nullptr;
    }

    created->SetOwner(owner);

    // Reciclado tem de nascer igual a um recém-construído: createProjectile
    // posiciona no dono, mas quem vem do pool não passa por ela.
    if (owner) {
        created->SetPosition(owner->GetPosition());
    }

    created->SetOriginFactory(this);

    return created;
}

void JulioProjectile1Factory::Release(std::unique_ptr<Projectile> projectile) {

    if (!dynamic_cast<JulioBossProjectile*>(projectile.get())) {
        SDL_Log("ERRO: JulioProjectile1Factory::Release recebeu um projetil de tipo incompativel!");
        return;
    }

    mPool.Release(std::unique_ptr<JulioBossProjectile>(
        dynamic_cast<JulioBossProjectile*>(projectile.release())
    ));
}

void JulioProjectile1Factory::Prewarm(Scene* scene, Actor* owner, int count) {

    std::vector<std::unique_ptr<Projectile>> held;
    held.reserve(count);

    for (int i = 0; i < count; ++i) {
        auto p = Acquire(scene, owner);
        if (!p) {
            SDL_Log("AVISO: JulioProjectile1Factory::Prewarm falhou ao criar instancia %d de %d.", i, count);
            continue;
        }
        held.push_back(std::move(p));
    }

    for (auto& p : held) {
        Release(std::move(p));
    }
}
