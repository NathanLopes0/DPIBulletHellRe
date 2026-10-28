//
// O disparo que sai de varios pontos do campo ao mesmo tempo.
//

#include "PontosAttack.h"

#include <SDL_log.h>

#include "../PathAim.h"
#include "../AttackParameters/PontosAttackParams.h"
#include "../../Actors/Projectile.h"
#include "../../Actors/ProjectileFactory.h"
#include "../../Components/RigidBodyComponent.h"

PontosAttack::PontosAttack(ProjectileFactory* spawner, Actor* owner)
    : IAttackStrategy(spawner, owner)
{
}

std::vector<std::unique_ptr<Projectile>> PontosAttack::Execute(const AttackParams& params) {

    std::vector<std::unique_ptr<Projectile>> projectiles;

    const auto* pontos = dynamic_cast<const PontosAttackParams*>(&params);
    if (!pontos) {
        SDL_Log("PontosAttack: os parametros nao sao PontosAttackParams - nada disparado");
        return projectiles;
    }
    if (params.numProjectiles <= 0) return projectiles;

    const float grau = Math::Pi / 180.0f;
    const int n = params.numProjectiles;
    const float passo = (n > 1) ? params.angle / static_cast<float>(n - 1) : 0.0f;

    projectiles.reserve(pontos->pontos.size() * static_cast<size_t>(n));
    for (const Vector2& de : pontos->pontos) {

        // CADA PONTO MIRA POR SI: oito pontos mirando o mesmo aluno sao oito
        // linhas que se fecham nele, e nao oito tiros paralelos.
        float centro = params.centralAngle;
        if (pontos->mirar && pontos->temAlvo) {
            const Vector2 v = pontos->prever ? pontos->velocidadeDoAlvo : Vector2::Zero;
            const Vector2 rumo = RumoPrevendo(de, pontos->alvo, v, params.projectileSpeed);
            centro = Math::ToDegrees(Math::Atan2(rumo.y, rumo.x));
        }

        const float inicio = centro - 0.5f * passo * static_cast<float>(n - 1);
        for (int i = 0; i < n; ++i) {
            auto projectile = Acquire();
            if (!projectile) continue;

            const float a = (inicio + passo * static_cast<float>(i)) * grau;
            projectile->SetPosition(de);
            if (const auto rb = projectile->GetComponent<RigidBodyComponent>()) {
                rb->SetVelocity(Vector2(Math::Cos(a), Math::Sin(a)) * params.projectileSpeed);
            }
            projectiles.push_back(std::move(projectile));
        }
    }
    return projectiles;
}
