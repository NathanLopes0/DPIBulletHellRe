//
// Created by gensh on 27/11/2025.
//

#pragma once
#include "BossProjectile.h"

/**
 * @class AndreBossProjectile
 * @brief Especialização do projétil do Andre.
 * A lógica é herdada de BossProjectile; esta classe existe
 * para que a Factory possa criar um tipo específico.
 */
class AndreBossProjectile : public BossProjectile {
public:
    explicit AndreBossProjectile(Scene* scene, Boss* owner);

    // Restaura a animação inicial ao ser reciclado pelo pool. createProjectile
    // sempre fixa "Graph1" após a criação, então replicamos o mesmo aqui.
    void Reset() override;
};
