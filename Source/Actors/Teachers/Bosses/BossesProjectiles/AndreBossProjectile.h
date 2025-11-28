//
// Created by gensh on 27/11/2025.
//

#pragma once
#include "BossProjectile.h"

/**
 * @class AndreBossProjectile
 * @brief Especialização do projétil do Andre (Grafos).
 * A lógica é herdada de BossProjectile; esta classe existe
 * para que a Factory possa criar um tipo específico.
 */
class AndreBossProjectile : public BossProjectile {
public:
    explicit AndreBossProjectile(Scene* scene, Boss* owner);
};
