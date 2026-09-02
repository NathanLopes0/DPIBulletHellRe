//
// Created by gensh on 26/11/2025.
//

#pragma once
#include "BossProjectile.h"
#include "../../../../Components/DrawComponents/DrawAnimatedComponent.h"


/**
 * @class RicardoBossProjectile
 * @brief Especialização do projétil do Ricardo (Arduíno).
 * A lógica é herdada de BossProjectile; esta classe existe
 * para que a Factory possa criar um tipo específico.
 */
class RicardoBossProjectile : public BossProjectile {
public:
    explicit RicardoBossProjectile(Scene* scene, Boss* owner)
        : BossProjectile(scene, owner) {}

    // Restaura a animação inicial ("Normal") ao ser reciclado pelo pool,
    // já que createProjectile também a define explicitamente na criação.
    void Reset() override {
        Projectile::Reset();
        if (auto drawComp = GetComponent<DrawAnimatedComponent>()) {
            drawComp->SetAnimation("Normal");
        }
    }
};
