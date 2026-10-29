//
// Uma palavra de bits, de lado a lado da tela.
//

#include "PalavraAttack.h"

#include <SDL_log.h>

#include "../Shared/Logica.h"
#include "../AttackParameters/PalavraAttackParams.h"
#include "../../Game.h"
#include "../../Actors/Projectile.h"
#include "../../Actors/ProjectileFactory.h"
#include "../../Actors/Teachers/Bosses/BossesProjectiles/BossProjectile.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Scenes/Scene.h"

PalavraAttack::PalavraAttack(ProjectileFactory* spawner, Actor* owner)
    : IAttackStrategy(spawner, owner)
{
}

std::vector<std::unique_ptr<Projectile>> PalavraAttack::Execute(const AttackParams& params) {

    std::vector<std::unique_ptr<Projectile>> projectiles;

    const auto* palavra = dynamic_cast<const PalavraAttackParams*>(&params);
    if (!palavra) {
        SDL_Log("PalavraAttack: os parametros nao sao PalavraAttackParams - nada disparado");
        return projectiles;
    }

    // ATE 31 BITS: a palavra mora num uint32_t, e o bit 31 seria o sinal de
    // quem a ler com int.
    const int bits = params.numProjectiles;
    if (bits <= 0 || bits > 31) return projectiles;

    const auto* cena = mOwner ? mOwner->GetScene() : nullptr;
    if (!cena || !cena->GetGame()) return projectiles;
    const auto largura = static_cast<float>(cena->GetGame()->GetWindowWidth());
    const float y = palavra->altura >= 0.0f ? palavra->altura : params.firePosition.y;

    projectiles.reserve(static_cast<size_t>(bits));
    for (int c = 0; c < bits; ++c) {
        auto bit = Acquire();
        if (!bit) continue;

        const bool um = Logica::Bit(palavra->palavra, c, bits);
        bit->SetPosition(Vector2(Logica::CentroDaColuna(c, largura, bits), y));
        if (auto* rb = bit->GetComponent<RigidBodyComponent>()) {
            rb->SetVelocity(Vector2(0.0f, params.projectileSpeed));
        }
        if (auto* anim = bit->GetComponent<DrawAnimatedComponent>()) {
            anim->SetAnimation(um ? palavra->um : palavra->zero);
        }
        if (auto* doChefe = dynamic_cast<BossProjectile*>(bit.get())) {
            doChefe->DefinirEnfeite(um ? palavra->umAtravessa : palavra->zeroAtravessa);
        }
        projectiles.push_back(std::move(bit));
    }
    return projectiles;
}
