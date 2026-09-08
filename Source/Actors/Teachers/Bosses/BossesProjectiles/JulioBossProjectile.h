//
// Projétil do Júlio (INF 420 - Inteligência Artificial)
//

#pragma once
#include "BossProjectile.h"
#include "../../../../Components/DrawComponents/DrawAnimatedComponent.h"

/**
 * @class JulioBossProjectile
 * @brief "Ponto de dado" disparado pelo Júlio.
 *
 * A lógica toda vem de BossProjectile. Esta classe existe para que a
 * ProjectilePool<T> da fábrica tenha um tipo concreto próprio — sem ela, os
 * projéteis do Júlio dividiriam pool com os de outro professor.
 *
 * As três animações registradas em createProjectile correspondem às três fases
 * da luta, e quem escolhe qual usar é Julio::OnProjectilesCreated.
 */
class JulioBossProjectile : public BossProjectile {
public:
    explicit JulioBossProjectile(Scene* scene, Boss* owner)
        : BossProjectile(scene, owner) {}

    // Ao voltar do pool, restaura a animação padrão. Sem isto, um projétil
    // reciclado da fase 3 apareceria na fase 1 ainda com a cor de "Overfit"
    // até OnProjectilesCreated corrigir — o que funciona, mas depende de outra
    // classe lembrar de fazer. Melhor o objeto se normalizar sozinho.
    void Reset() override {
        Projectile::Reset();
        if (auto drawComp = GetComponent<DrawAnimatedComponent>()) {
            drawComp->SetAnimation("Ruido");
        }
    }
};
