//
// Created by nslop on 21/11/2024.
//
#pragma once

#include "BossProjectile.h"

class Scene;
class Boss;

/**
 * @class SallesBossProjectile
 * @brief Especialização do projétil do Salles (Capivara).
 * A lógica é herdada de BossProjectile; esta classe existe
 * para que a Factory possa criar um tipo específico.
 */
class SallesBossProjectile : public BossProjectile {
public:
    /**
     * @brief O construtor "carteiro".
     * Ele apenas recebe os argumentos e os repassa
     * para o construtor da classe pai.
     */
    explicit SallesBossProjectile(Scene* scene, Boss* owner)
        : BossProjectile(scene, owner) {} // "Implementação" do construtor já aqui, apenas
                                            // repassando os argumento para o BossProjectile.
};
