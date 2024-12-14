//
// Created by nslop on 17/09/2024.
//

#ifndef DPIBULLETHELLRE_BOSSFACTORY_H
#define DPIBULLETHELLRE_BOSSFACTORY_H

#include <vector>
#include <string>
#include <memory>
#include "../../../AttackStrategy.h"

class BossFactory {

public:
    explicit BossFactory(class Game* game);
    virtual class Boss* CreateBoss(class Scene*) = 0;
protected:

    std::vector<AttackStrategy*> mAtkStrategies;
    std::vector<std::string> spritePathing;
    std::vector<std::string> dataPathing;

    class Boss* mBoss;
    class Game* mGame;


    virtual void configureBaseDrawComponent(class DrawAnimatedComponent* drawAnimatedComponent);
    virtual void configureBossSprite() = 0;
    virtual void configureStrategies() = 0;

};


#endif //DPIBULLETHELLRE_BOSSFACTORY_H
