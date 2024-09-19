//
// Created by nslop on 17/09/2024.
//

#ifndef DPIBULLETHELLRE_BOSSFACTORY_H
#define DPIBULLETHELLRE_BOSSFACTORY_H


class BossFactory {

public:
    BossFactory(class Game* game);
    virtual void CreateBoss() = 0;
protected:
    class Boss* mBoss;
    class Game* mGame;

};


#endif //DPIBULLETHELLRE_BOSSFACTORY_H
