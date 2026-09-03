//
// Created by gensh on 09/04/2026.
//

#ifndef DPIBULLETHELLRE_SALLESDOUBLELISTPROJECTILEFACTORY_H
#define DPIBULLETHELLRE_SALLESDOUBLELISTPROJECTILEFACTORY_H
#include "../../../ProjectileFactory.h"
#include "../../../ProjectilePool.h"
#include "../../Bosses/BossesProjectiles/SallesDoubleListProjectile.h"


class SallesDoubleListProjectileFactory : public ProjectileFactory {


public:

    SallesDoubleListProjectileFactory()
        : ProjectileFactory("../Assets/Teachers/Projectiles/DPIBHSallesDuplamente.png",
                            "../Assets/Teachers/Projectiles/DPIBHSallesDuplamente.json") {}

    std::unique_ptr<Projectile> createProjectile(Scene *scene, Actor *owner) override;

    std::unique_ptr<Projectile> Acquire(Scene* scene, Actor* owner) override;
    void Release(std::unique_ptr<Projectile> projectile) override;
    void Prewarm(Scene* scene, Actor* owner, int count) override;

private:
    ProjectilePool<SallesDoubleListProjectile> mPool;

};



#endif //DPIBULLETHELLRE_SALLESDOUBLELISTPROJECTILEFACTORY_H
