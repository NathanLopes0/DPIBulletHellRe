//
// Created by gensh on 01/04/2026.
//

#include "LaserAttack.h"

#include <SDL_log.h>

#include "../../Actors/Projectile.h"
#include "../../Scenes/Battle/Battle.h"
#include "../../Actors/ProjectileFactory.h"
#include "../../Actors/Player/Player.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"


class DrawAnimatedComponent;

LaserAttack::LaserAttack(ProjectileFactory *spawner, Actor *owner)
    : mSpawner(spawner),
    mOwner(owner)
{

}

std::vector<std::unique_ptr<Projectile> > LaserAttack::Execute(const AttackParams &params) {

    std::vector<std::unique_ptr<Projectile>> projectiles;

    const int numProjectiles = params.numProjectiles;
    const float projectileSpeed = params.projectileSpeed;

    const auto basePrototypeProjectile = mSpawner->createProjectile(mOwner->GetScene(), mOwner);

    for (int i = 0; i < numProjectiles; i++) {

        auto projectile = mSpawner->createProjectile(mOwner->GetScene(), mOwner);
        if (!projectile) {
            SDL_Log("Failed to create projectile");
            continue;
        }

        // Projectile começa na posicao do boss, exceto se o params.FirePosition tiver algo. Ai ele nasce onde está definido na configuração
        if (params.firePosition.x != 0 && params.firePosition.y != 0) {
            projectile->SetPosition(params.firePosition);
        }

        // Lógica de direção e laser

        //Começa invisivel
        auto drawComp = projectile->GetComponent<DrawAnimatedComponent>();

        //Começa parado
        projectile->GetComponent<RigidBodyComponent>()->SetVelocity(Vector2::Zero);

        const int sH = drawComp->GetSpriteHeight();
        float timeToSpawn = static_cast<float>(sH) / projectileSpeed;

        //Insere o comportamento de ativação e homing
        projectile->insertBehavior<ActivateBehavior>(i * timeToSpawn);
        projectile->insertBehavior<HomingBehavior>(i * timeToSpawn, projectileSpeed);

        auto battle = dynamic_cast<Battle*>(mOwner->GetScene());
        auto pX = battle->GetPlayer()->GetPosition().x;
        auto pY = battle->GetPlayer()->GetPosition().y;
        auto degreesToPlayer = Math::ToDegrees(Math::Atan2(projectile->GetPosition(). y - pY, projectile->GetPosition().x - pX));
        SDL_Log("%f", degreesToPlayer);
        projectile->SetRotation(degreesToPlayer);

        projectiles.push_back(std::move(projectile));
    }

    return projectiles;
}
