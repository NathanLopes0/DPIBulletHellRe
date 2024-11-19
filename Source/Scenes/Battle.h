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

    explicit Battle(Game* game);
    void Load() override;
    [[nodiscard]] class Boss* GetBoss() const override { return mBoss; };

private:
    float mGrade;
    class BossFactory* mBossFactory;
    class Boss* mBoss;
    class Player* mPlayer;
    void BattleStart();

    void LoadHUD();

    void LoadPlayer();
};


#endif //DPIBULLETHELLRE_BATTLE_H
