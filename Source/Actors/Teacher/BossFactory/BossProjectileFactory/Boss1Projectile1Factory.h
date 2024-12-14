//
// Created by nslop on 10/12/2024.
//

#ifndef DPIBULLETHELLRE_BOSS1PROJECTILE1FACTORY_H
#define DPIBULLETHELLRE_BOSS1PROJECTILE1FACTORY_H


#include "../../../ProjectileFactory.h"

class Boss1Projectile1Factory : public ProjectileFactory {
public:
    Projectile* createProjectile(class Scene* scene, class Boss* owner) override;

private:
    std::string mSpritePath = "../Assets/Teachers/Projectiles/Capivara.png";
    std::string mDataPath = "../Assets/Teachers/Projectiles/Capivara.json";
};


#endif //DPIBULLETHELLRE_BOSS1PROJECTILE1FACTORY_H
