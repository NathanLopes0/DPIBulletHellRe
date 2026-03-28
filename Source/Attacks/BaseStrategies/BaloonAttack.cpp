//
// Created by gensh on 20/03/2026.
//

#include "BaloonAttack.h"

#include <SDL_log.h>

#include "../../Actors/Projectile.h"
#include "../AttackParameters/AttackParams.h"
#include "../AttackParameters/BaloonAttackParams.h"
#include "../../Actors/ProjectileFactory.h"
#include "../../Actors/Player/Player.h"
#include "../../Scenes/Battle/Battle.h"

BaloonAttack::BaloonAttack(ProjectileFactory* projectileFactory, Actor* owner)
    : mSpawner(projectileFactory),
      mOwner(owner)
{

}

std::vector<std::unique_ptr<Projectile> > BaloonAttack::Execute(const AttackParams &params) {

    if (const auto baloonParams = dynamic_cast<const BaloonAttackParams*>(&params)) {

        // retorno final da função, que vai ser populado
        std::vector<std::unique_ptr<Projectile>> projectiles;

        const int numProjectiles = params.numProjectiles;
        const float projectileSpeed = params.projectileSpeed;

        if (numProjectiles == 0) {
            return projectiles;
        }


        // Reserva espaço no vetor para eficiência
        projectiles.reserve(numProjectiles);

        //lógica matemática
        if (baloonParams->randomSpawn) {
            if (baloonParams->centerOnPlayer) {
                for (int i = 0; i < numProjectiles; i++) {

                    auto projectile = mSpawner->createProjectile(mOwner->GetScene(), mOwner);

                    // Failsafe: Se a fábrica falhar, pula este projétil
                    if (!projectile) {
                        SDL_Log("Failed to create projectile");
                        continue;
                    }

                    const auto currScene = mOwner->GetScene();

                    if (baloonParams->side == BaloonAttackParams::side::Down) {
                        if (const auto battle = dynamic_cast<Battle*>(currScene)) {
                            float finalXPosition = Random::GetFloatRange(-baloonParams->centerOnPlayerOffset,
                                                                                baloonParams->centerOnPlayerOffset);

                            finalXPosition += battle->GetPlayer()->GetPosition().x;

                            projectile->SetPosition(Vector2(finalXPosition,
                                mOwner->GetScene()->GetGame()->GetWindowHeight()
                                        + projectile->GetComponent<DrawAnimatedComponent>()->GetSpriteHeight()));

                            auto activateDelay = Random::GetFloatRange(0, 3);
                            projectile->insertBehavior<ActivateBehavior>(activateDelay, Vector2(0,
                                                                                        -params.projectileSpeed));

                        }
                    }

                    projectiles.push_back(std::move(projectile));

                }
            }
        }

        return projectiles;
    }

    SDL_Log("Retornando vetor vazio de projeteis");
    // Se os parâmetros passados não forem do tipo especial BaloonAttackParams, retorna um vetor sem projéteis
    return {};
}
