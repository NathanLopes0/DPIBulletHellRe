//
// A caixa do HTML: um retangulo de projeteis que nasce em volta do chefe e
// cresce sem perder a forma.
//

#include "CaixaAttack.h"

#include <SDL_log.h>

#include "../Shared/Caixa.h"
#include "../AttackParameters/CaixaAttackParams.h"
#include "../../Actors/Projectile.h"
#include "../../Actors/ProjectileFactory.h"
#include "../../Components/RigidBodyComponent.h"

CaixaAttack::CaixaAttack(ProjectileFactory* spawner, Actor* owner)
    : IAttackStrategy(spawner, owner)
{
}

std::vector<std::unique_ptr<Projectile>> CaixaAttack::Execute(const AttackParams& params) {

    std::vector<std::unique_ptr<Projectile>> projectiles;

    const auto* caixa = dynamic_cast<const CaixaAttackParams*>(&params);
    if (!caixa) {
        SDL_Log("CaixaAttack: os parametros nao sao CaixaAttackParams - nada disparado");
        return projectiles;
    }

    const float meiaAltura  = caixa->meiaAlturaInicial;
    const float meiaLargura = meiaAltura * caixa->proporcao;
    const auto pontos = Caixa::PontosDoContorno(params.numProjectiles, meiaLargura, meiaAltura);

    projectiles.reserve(pontos.size());
    for (const Vector2& ponto : pontos) {

        auto projectile = Acquire();
        if (!projectile) continue;

        // NASCE NO CONTORNO E JA ANDANDO. Sem DeactivateBehavior: a caixa tem de
        // aparecer inteira no primeiro quadro, ou a div seria lida aos pedacos.
        projectile->SetPosition(params.firePosition + ponto);
        if (const auto rb = projectile->GetComponent<RigidBodyComponent>()) {
            rb->SetVelocity(Caixa::Velocidade(ponto, meiaAltura, params.projectileSpeed));
        }

        projectiles.push_back(std::move(projectile));
    }

    return projectiles;
}
