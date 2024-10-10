//
// Created by nslop on 16/09/2024.
//

#ifndef DPIBULLETHELLRE_BATTLE_H
#define DPIBULLETHELLRE_BATTLE_H

#include "Scene.h"
#include "../Game.h"
#include "../Actors/Teacher/BossFactory/BossFactory.h"

class Battle : public Scene {

public:

    Battle(Game* game);
    void Load();

private:
    float mGrade;
    class BossFactory* mBossFactory;
    class Boss* mBoss;

    void LoadHUD();
};


#endif //DPIBULLETHELLRE_BATTLE_H
