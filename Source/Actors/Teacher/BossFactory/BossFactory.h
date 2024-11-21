//
// Created by nslop on 17/09/2024.
//

#ifndef DPIBULLETHELLRE_BOSSFACTORY_H
#define DPIBULLETHELLRE_BOSSFACTORY_H


class BossFactory {

public:
    explicit BossFactory(class Game* game);
    virtual class Boss* CreateBoss(class Scene*) = 0;
protected:
    class Boss* mBoss;
    class Game* mGame;

};


#endif //DPIBULLETHELLRE_BOSSFACTORY_H
