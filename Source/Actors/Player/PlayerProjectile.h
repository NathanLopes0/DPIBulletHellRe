//
// Created by nslop on 19/11/2024.
//

#ifndef DPIBULLETHELLRE_PLAYERPROJECTILE_H
#define DPIBULLETHELLRE_PLAYERPROJECTILE_H

#include "../Projectile.h"

class PlayerProjectile : public Projectile {

    friend class Player;

public:
    PlayerProjectile(class Scene* scene, class Player* owner);
    void OnUpdate(float deltaTime) override;
    void OnCollision() override;
    [[nodiscard]] bool InsideProjectileLimit() const override;


private:

    class Player* mOwner;
    const CircleColliderComponent *BossCollider();

    void ActivateProjectile();
    void DeactivateProjectile() override;

    bool Collided();

    bool CheckCollision();
};


#endif //DPIBULLETHELLRE_PLAYERPROJECTILE_H
