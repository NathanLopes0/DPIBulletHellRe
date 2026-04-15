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

        //Começa indo em direção ao jogador
        const int sH = drawComp->GetSpriteHeight();
        float timeToSpawn = static_cast<float>(sH) / projectileSpeed;




        auto battle = dynamic_cast<Battle*>(mOwner->GetScene());
        auto pX = battle->GetPlayer()->GetPosition().x;
        auto pY = battle->GetPlayer()->GetPosition().y;
        auto degreesToPlayer = Math::ToDegrees(Math::Atan2(projectile->GetPosition(). y - pY, projectile->GetPosition().x - pX));
        projectile->SetRotation(degreesToPlayer);

        // TODO - dar um jeito do laser ir todos os projeteis em uma direção só.
        // TODO - tentar dar uma direção, desativar, e depois ativar dnovo usando a direção anterior

        //Insere o comportamento de ativação
        auto pDirection = projectile->GetForward();
        pDirection.Normalize();

        // TODO - ler o comentario abaixo dnovo
        // ao inserir o Activate Behavior, o comportamento desativa o projétil até que chegue o tempo ativado. Lembrar disso...
        // talvez o Activate Behavior deveria apenas ativar, e eu desativar manualmente tbm... então cada Activate viria com um Deactivate antes,
        // apesar de eu escrever mais, o Activate faria mais sentido com oq ele deveria apenas fazer, e pode acabar sendo mais personalizavel.
        projectile->insertBehavior<ActivateBehavior>(i * timeToSpawn, pDirection * projectileSpeed);

        projectiles.push_back(std::move(projectile));
    }

    return projectiles;
}
