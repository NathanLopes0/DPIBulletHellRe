//
// A sprint dos metodos ageis: o cursor arrasta as historias para o quadro.
//

#include "SprintAttack.h"

#include <SDL_log.h>

#include "../Behaviors.h"
#include "../Sprint.h"
#include "../AttackParameters/SprintAttackParams.h"
#include "../../Actors/Projectile.h"
#include "../../Actors/ProjectileFactory.h"
#include "../../Actors/Teachers/Bosses/BossesProjectiles/BossProjectile.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Components/RigidBodyComponent.h"

SprintAttack::SprintAttack(ProjectileFactory* spawner, Actor* owner)
    : IAttackStrategy(spawner, owner)
{
}

namespace {
    void Vestir(Projectile* p, const std::string& cara) {
        if (cara.empty()) return;
        if (auto* anim = p->GetComponent<DrawAnimatedComponent>()) anim->SetAnimation(cara);
    }
}

std::vector<std::unique_ptr<Projectile>> SprintAttack::Execute(const AttackParams& params) {

    std::vector<std::unique_ptr<Projectile>> projectiles;

    const auto* sprint = dynamic_cast<const SprintAttackParams*>(&params);
    if (!sprint) {
        SDL_Log("SprintAttack: os parametros nao sao SprintAttackParams - nada disparado");
        return projectiles;
    }
    if (params.numProjectiles <= 0) return projectiles;

    // O PLANO E DESTA SPRINT: o cursor e as historias dela leem o mesmo, e
    // duas sprints em campo nao se confundem.
    const auto plano = std::make_shared<const Sprint::Plano>(
        Sprint::Planejar(params.firePosition, params.numProjectiles, sprint->revisao));

    projectiles.reserve(static_cast<size_t>(params.numProjectiles) + 1);
    for (int i = 0; i < params.numProjectiles; ++i) {
        auto historia = Acquire();
        if (!historia) continue;
        historia->SetPosition(Sprint::PosicaoDaHistoria(*plano, i, 0.0f));
        if (const auto rb = historia->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
        Vestir(historia.get(), sprint->historia);
        // NA MAO DO CURSOR, BLINDADA: o tiro some na pilha. O behavior a
        // solta ao soltar a pilha.
        if (auto* bp = dynamic_cast<BossProjectile*>(historia.get())) bp->DefinirBlindado(true);
        historia->insertMotion<HistoriaDaSprintBehavior>(plano, i, params.projectileSpeed);
        projectiles.push_back(std::move(historia));
    }

    // O CURSOR VEM POR ULTIMO, e so se houver historia para arrastar.
    if (projectiles.empty()) return projectiles;
    if (auto cursor = Acquire()) {
        if (auto* mao = dynamic_cast<BossProjectile*>(cursor.get())) mao->DefinirEnfeite(true);
        cursor->SetPosition(Sprint::CentroDoCursor(plano->origem, cursor->GetScale()));
        if (const auto rb = cursor->GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
        Vestir(cursor.get(), sprint->cursor);
        cursor->insertMotion<CursorDaSprintBehavior>(plano, sprint->cursor, sprint->clique);
        projectiles.push_back(std::move(cursor));
    }
    return projectiles;
}
