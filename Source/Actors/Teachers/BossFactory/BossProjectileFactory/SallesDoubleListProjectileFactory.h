//
// Created by gensh on 09/04/2026.
//

#ifndef DPIBULLETHELLRE_SALLESDOUBLELISTPROJECTILEFACTORY_H
#define DPIBULLETHELLRE_SALLESDOUBLELISTPROJECTILEFACTORY_H
#include "../../../ProjectileFactory.h"


class SallesDoubleListProjectileFactory : public ProjectileFactory {


public:
    std::unique_ptr<Projectile> createProjectile(Scene *scene, Actor *owner) override;

};



#endif //DPIBULLETHELLRE_SALLESDOUBLELISTPROJECTILEFACTORY_H
