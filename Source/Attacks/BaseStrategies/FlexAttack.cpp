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

    // O EIXO PRINCIPAL: x numa fileira, y numa coluna.
    const float meio   = flex->coluna ? params.firePosition.y : params.firePosition.x;
    const float inicio = meio - flex->largura / 2.0f;
    const float fim    = meio + flex->largura / 2.0f;
    const auto antes  = Flex::Posicoes(flex->de,   params.numProjectiles, flex->item, inicio, fim);
    const auto depois = Flex::Posicoes(flex->para, params.numProjectiles, flex->item, inicio, fim);

    projectiles.reserve(antes.size());
    for (size_t i = 0; i < antes.size() && i < depois.size(); ++i) {

        auto projectile = Acquire();
        if (!projectile) continue;

        // NASCE TODA NO MESMO QUADRO, ja andando: um arranjo que aparece aos
        // pedacos nao se le como arranjo. A FILEIRA nasce na altura do chefe e
        // cai; a COLUNA nasce na borda e atravessa a tela de lado.
        const Vector2 lugar = flex->coluna ? Vector2(params.firePosition.x, antes[i])
                                           : Vector2(antes[i], params.firePosition.y);
        const Vector2 voo = flex->coluna ? Vector2(flex->sentido * params.projectileSpeed, 0.0f)
                                         : Vector2(0.0f, params.projectileSpeed);
        const Vector2 rearranjo = flex->coluna ? Vector2(0.0f, depois[i] - antes[i])
                                               : Vector2(depois[i] - antes[i], 0.0f);
        projectile->SetPosition(lugar);
        if (const auto rb = projectile->GetComponent<RigidBodyComponent>()) rb->SetVelocity(voo);

        projectile->insertModifier<DeslocarBehavior>(flex->aos, flex->em, rearranjo);

        projectiles.push_back(std::move(projectile));
    }

    return projectiles;
}
