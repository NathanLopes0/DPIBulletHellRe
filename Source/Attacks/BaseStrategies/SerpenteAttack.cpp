//
// A serpente dos sequenciadores: a cabeca persegue, o corpo segue.
//

#include "SerpenteAttack.h"

#include <SDL_log.h>

#include "../Behaviors.h"
#include "../AttackParameters/SerpenteAttackParams.h"
#include "../../Actors/Projectile.h"
#include "../../Actors/ProjectileFactory.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Components/RigidBodyComponent.h"

SerpenteAttack::SerpenteAttack(ProjectileFactory* spawner, Actor* owner)
    : IAttackStrategy(spawner, owner)
{
}

std::vector<std::unique_ptr<Projectile>> SerpenteAttack::Execute(const AttackParams& params) {

    std::vector<std::unique_ptr<Projectile>> projectiles;

    const auto* serpente = dynamic_cast<const SerpenteAttackParams*>(&params);
    if (!serpente) {
        SDL_Log("SerpenteAttack: os parametros nao sao SerpenteAttackParams - nada disparado");
        return projectiles;
    }
    if (params.numProjectiles <= 0) return projectiles;

    const float grau = Math::Pi / 180.0f;
    const Vector2 voo(Math::Cos(params.centralAngle * grau) * params.projectileSpeed,
                      Math::Sin(params.centralAngle * grau) * params.projectileSpeed);

    // O CAMINHO E DESTA SERPENTE, e de mais ninguem: duas serpentes na tela nao
    // podem andar pelo rastro uma da outra.
    auto rastro = std::make_shared<Rastro::Serpente>();

    projectiles.reserve(static_cast<size_t>(params.numProjectiles));
    for (int i = 0; i < params.numProjectiles; ++i) {

        auto projectile = Acquire();
        if (!projectile) continue;

        projectile->SetPosition(params.firePosition);
        if (const auto rb = projectile->GetComponent<RigidBodyComponent>()) rb->SetVelocity(voo);

        const std::string& cara = (i == 0) ? serpente->cabeca : serpente->corpo;
        if (!cara.empty()) {
            if (auto* anim = projectile->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(cara);
        }

        if (i == 0) {
            projectile->insertMotion<CabecaDaSerpenteBehavior>(rastro, serpente->desvio,
                                                               serpente->giro, serpente->duracao);
        } else {
            projectile->insertMotion<GomoDaSerpenteBehavior>(rastro, serpente->espaco * static_cast<float>(i));
        }

        projectiles.push_back(std::move(projectile));
    }

    return projectiles;
}
