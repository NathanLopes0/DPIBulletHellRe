//
// Created by nslop on 21/11/2024.
//

#pragma once

#include "../../../Projectile.h"

class Boss;

/**
 * @class BossProjectile
 * @brief Uma especialização de Projétil que é disparada por um Boss.
 * Responsável por carregar seus componentes específicos e atualizar seus Behaviors.
 */
class BossProjectile : public Projectile {
public:
    explicit BossProjectile(Scene* scene, Boss* owner);
    ~BossProjectile() override = default;

    void OnUpdate(float deltaTime) override;

    [[nodiscard]] Boss* GetBossOwner() const;

    Vector2 GetPlayerPosition() const;

    Vector2 GetPlayerDirection() const;

protected:
    // Implementação obrigatória do contrato da classe base.
    [[nodiscard]] bool IsOffScreen() const override;

};
