//
// O flexbox do CSS: uma fileira que cai arrumada de um jeito e, no meio da
// queda, se rearranja de outro.
//

#include "FlexAttack.h"

#include <SDL_log.h>

#include "../Behaviors.h"
#include "../Flex.h"
#include "../AttackParameters/FlexAttackParams.h"
#include "../../Actors/Projectile.h"
#include "../../Actors/ProjectileFactory.h"
#include "../../Components/RigidBodyComponent.h"

FlexAttack::FlexAttack(ProjectileFactory* spawner, Actor* owner)
    : IAttackStrategy(spawner, owner)
{
}

std::vector<std::unique_ptr<Projectile>> FlexAttack::Execute(const AttackParams& params) {

    std::vector<std::unique_ptr<Projectile>> projectiles;

    const auto* flex = dynamic_cast<const FlexAttackParams*>(&params);
    if (!flex) {
        SDL_Log("FlexAttack: os parametros nao sao FlexAttackParams - nada disparado");
        return projectiles;
    }

    const float inicio = params.firePosition.x - flex->largura / 2.0f;
    const float fim    = params.firePosition.x + flex->largura / 2.0f;
    const auto antes  = Flex::Posicoes(flex->de,   params.numProjectiles, flex->item, inicio, fim);
    const auto depois = Flex::Posicoes(flex->para, params.numProjectiles, flex->item, inicio, fim);

    projectiles.reserve(antes.size());
    for (size_t i = 0; i < antes.size() && i < depois.size(); ++i) {

        auto projectile = Acquire();
        if (!projectile) continue;

        // NASCE NA ALTURA DO CHEFE, JA CAINDO, e a fileira inteira no mesmo
        // quadro: um arranjo que aparece aos pedacos nao se le como arranjo.
        projectile->SetPosition(Vector2(antes[i], params.firePosition.y));
        if (const auto rb = projectile->GetComponent<RigidBodyComponent>()) {
            rb->SetVelocity(Vector2(0.0f, params.projectileSpeed));
        }

        projectile->insertModifier<DeslocarBehavior>(flex->aos, flex->em, depois[i] - antes[i]);

        projectiles.push_back(std::move(projectile));
    }

    return projectiles;
}
