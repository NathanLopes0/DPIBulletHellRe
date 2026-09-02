//
// Created by nslop on 21/11/2024.
//
#pragma once

#include "BossProjectile.h"
#include "../../../../Components/DrawComponents/DrawAnimatedComponent.h"

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

    // Restaura a animação "Normal" (estado de fábrica) ao ser reciclado.
    // A troca para "Homing" continua acontecendo via o configurator de
    // AddAttackPattern, chamado logo após Acquire() em Boss::ExecuteAttack —
    // isso não muda com o pooling, então não precisa ser tratado aqui.
    void Reset() override {
        Projectile::Reset();
        if (auto drawComp = GetComponent<DrawAnimatedComponent>()) {
            drawComp->SetAnimation("Normal");
        }
    }
};
