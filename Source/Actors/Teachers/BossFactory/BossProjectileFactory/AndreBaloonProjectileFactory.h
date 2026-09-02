//
// Created by gensh on 24/03/2026.
//

#ifndef DPIBULLETHELLRE_ANDREBALOONPROJECTILEFACTORY_H
#define DPIBULLETHELLRE_ANDREBALOONPROJECTILEFACTORY_H
#include "../../../ProjectileFactory.h"
#include "../../../ProjectilePool.h"
#include "../../Bosses/BossesProjectiles/AndreBaloonProjectile.h"


class AndreBaloonProjectileFactory : public ProjectileFactory {

public:

    std::unique_ptr<Projectile> createProjectile(Scene *scene, Actor *owner) override;

    std::unique_ptr<Projectile> Acquire(Scene* scene, Actor* owner) override;
    void Release(std::unique_ptr<Projectile> projectile) override;
    void Prewarm(Scene* scene, Actor* owner, int count) override;

private:

    // Pool exclusivo de AndreBaloonProjectile: reciclar só faz sentido entre
    // objetos do mesmo tipo concreto (mesmos Components, mesma classe).
    ProjectilePool<AndreBaloonProjectile> mPool;

};


#endif //DPIBULLETHELLRE_ANDREBALOONPROJECTILEFACTORY_H