//
// Created by nslop on 10/12/2024.
//

#ifndef DPIBULLETHELLRE_PROJECTILEFACTORY_H
#define DPIBULLETHELLRE_PROJECTILEFACTORY_H


#include <string>
#include "Projectile.h"

class ProjectileFactory {

public:
    virtual ~ProjectileFactory() = default;
    virtual Projectile* createProjectile(class Scene*, class Boss*) = 0;

protected:
    virtual void configureBaseDrawComponent(class DrawAnimatedComponent*);

};


#endif //DPIBULLETHELLRE_PROJECTILEFACTORY_H
