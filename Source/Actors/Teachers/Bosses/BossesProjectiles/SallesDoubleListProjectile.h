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
};



#endif //DPIBULLETHELLRE_SALLESDOUBLELISTPROJECTILE_H
