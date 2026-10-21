//
// A capsula do encapsulamento: as letras viajam fechadas e abrem num anel.
//

#include "CapsulaAttack.h"

#include <SDL_log.h>

#include "../Behaviors.h"
#include "../Capsula.h"
#include "../AttackParameters/CapsulaAttackParams.h"
#include "../../Actors/Projectile.h"
#include "../../Actors/ProjectileFactory.h"
#include "../../Actors/Teachers/Bosses/BossesProjectiles/BossProjectile.h"
#include "../../Components/RigidBodyComponent.h"

CapsulaAttack::CapsulaAttack(ProjectileFactory* spawner, Actor* owner)
    : IAttackStrategy(spawner, owner)
{
}

std::vector<std::unique_ptr<Projectile>> CapsulaAttack::Execute(const AttackParams& params) {

    std::vector<std::unique_ptr<Projectile>> projectiles;

    const auto* capsula = dynamic_cast<const CapsulaAttackParams*>(&params);
    if (!capsula) {
        SDL_Log("CapsulaAttack: os parametros nao sao CapsulaAttackParams - nada disparado");
        return projectiles;
    }

    const float kGrau = Math::Pi / 180.0f;
    const Vector2 voo(Math::Cos(params.centralAngle * kGrau) * params.projectileSpeed,
                      Math::Sin(params.centralAngle * kGrau) * params.projectileSpeed);
    const auto anel = Capsula::Abertura(params.numProjectiles, params.centralAngle,
                                        capsula->velocidadeDaAbertura);

    projectiles.reserve(anel.size());
    for (const Vector2& abertura : anel) {

        auto projectile = Acquire();
        if (!projectile) continue;

        // TODAS NO MESMO PONTO E COM A MESMA VELOCIDADE: a pilha e a capsula.
        projectile->SetPosition(params.firePosition);
        if (const auto rb = projectile->GetComponent<RigidBodyComponent>()) {
            rb->SetVelocity(voo);
        }
        if (auto* letra = dynamic_cast<BossProjectile*>(projectile.get())) {
            letra->DefinirBlindado(true);
        }

        // A escala de AGORA e a de fabrica (o pool a devolveu assim) e e o
        // tamanho do que sai da capsula; a regra da rajada, que roda depois
        // daqui, pode aumentar a casca.
        projectile->insertModifier<AbrirCapsulaBehavior>(capsula->abre, abertura, capsula->escreve,
                                                         projectile->GetScale());

        projectiles.push_back(std::move(projectile));
    }

    return projectiles;
}
