//
// Created by gensh on 09/04/2026.
//

#ifndef DPIBULLETHELLRE_SALLESDOUBLELISTPROJECTILE_H
#define DPIBULLETHELLRE_SALLESDOUBLELISTPROJECTILE_H
#include "BossProjectile.h"
#include "../../../../Components/DrawComponents/DrawAnimatedComponent.h"

/**
 * @class SallesDoubleListProjectile
 * @brief Especialização do projétil do Salles (Lista Duplamente Encadeada).
 * A lógica é herdada de BossProjectile; esta classe existe
 * para que a Factory possa criar um tipo específico.
 */

class DrawAnimatedComponent;

class SallesDoubleListProjectile : public BossProjectile {

public:
    explicit SallesDoubleListProjectile(Scene* scene, Boss* owner) : BossProjectile(scene, owner) {};

    // Restaura a animação inicial ("Normal") ao ser reciclado pelo pool,
    // já que createProjectile também a define explicitamente na criação.
    void Reset() override {
        Projectile::Reset();
        if (auto drawComp = GetComponent<DrawAnimatedComponent>()) {
            drawComp->SetAnimation("Normal");
        }
    }
};



#endif //DPIBULLETHELLRE_SALLESDOUBLELISTPROJECTILE_H
