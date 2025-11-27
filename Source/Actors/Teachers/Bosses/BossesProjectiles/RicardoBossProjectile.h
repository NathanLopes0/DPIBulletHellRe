//
// Created by gensh on 26/11/2025.
//

#pragma once
#include "BossProjectile.h"


class Scene;
class Boss;

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
};
