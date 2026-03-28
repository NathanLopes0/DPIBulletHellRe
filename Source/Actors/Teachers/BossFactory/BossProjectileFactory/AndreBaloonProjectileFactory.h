//
// Created by gensh on 24/03/2026.
//

#ifndef DPIBULLETHELLRE_ANDREBALOONPROJECTILEFACTORY_H
#define DPIBULLETHELLRE_ANDREBALOONPROJECTILEFACTORY_H
#include "../../../ProjectileFactory.h"


class AndreBaloonProjectileFactory : public ProjectileFactory {

public:

    std::unique_ptr<Projectile> createProjectile(Scene *scene, Actor *owner) override;

};


#endif //DPIBULLETHELLRE_ANDREBALOONPROJECTILEFACTORY_H