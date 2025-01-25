//
// Created by nslop on 16/09/2024.
//

#ifndef DPIBULLETHELLRE_BATTLE_H
#define DPIBULLETHELLRE_BATTLE_H

#include <memory>
#include "Scene.h"
#include "../Game.h"
#include "../Actors/Teacher/BossFactory/BossFactory.h"

class Battle : public Scene {

public:

    explicit Battle(Game* game);
    void Load() override;
    [[nodiscard]] Boss* GetBoss() const override { return mBoss.get(); };
    const Vector2& GetPlayerPosition() override;
    std::vector<Projectile*> GetBossProjectiles() override { return mBossAllProjectiles; };

    // Update ta aqui pra debug por enquanto
    void Update(float deltaTime) override;


private:
    int mBossNum;
    class BossFactory* mBossFactory;
    std::unique_ptr<class Boss> mBoss;
    std::unique_ptr<class Player> mPlayer;
    void BattleStart();

    void LoadHUD();

    void LoadPlayer();
};


#endif //DPIBULLETHELLRE_BATTLE_H
