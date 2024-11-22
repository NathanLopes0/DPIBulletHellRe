//
// Created by nslop on 21/11/2024.
//

#ifndef DPIBULLETHELLRE_BOSSPROJECTILE_H
#define DPIBULLETHELLRE_BOSSPROJECTILE_H

#include "../../../Projectile.h"


class BossProjectile : public Projectile {

public:
    BossProjectile(class Scene* scene, class Boss* owner);

    void OnUpdate(float deltaTime) override;

    void DeactivateProjectile() override;

protected:
    [[nodiscard]] bool InsideProjectileLimit() const override;
    bool CheckCollision();
private:
    class Boss* mOwner;
};


#endif //DPIBULLETHELLRE_BOSSPROJECTILE_H
