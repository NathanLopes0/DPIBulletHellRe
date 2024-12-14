//
// Created by nslop on 25/09/2024.
//

#ifndef DPIBULLETHELLRE_BOSS1FACTORY_H
#define DPIBULLETHELLRE_BOSS1FACTORY_H

#include "BossFactory.h"
#include "BossProjectileFactory/Boss1Projectile1Factory.h"

class Boss1Factory : public BossFactory {

public:
    explicit Boss1Factory(class Game* game);
    Boss * CreateBoss(class Scene*) override;

private:

    void configureStrategies() override;
    void configureBossSprite() override;
    Boss1Projectile1Factory* mProjectileSpawner;
};


#endif //DPIBULLETHELLRE_BOSS1FACTORY_H
