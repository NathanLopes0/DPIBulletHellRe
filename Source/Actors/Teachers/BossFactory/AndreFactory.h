//
// Created by gensh on 27/11/2025.
//

#pragma once
#include "IBossFactory.h"

class AndreFactory : public IBossFactory {
public:
    explicit AndreFactory(Game* game);
    ~AndreFactory() override = default;

private:

    // Os 3 passos da função principal CreateBoss da classe base
    std::unique_ptr<Boss> InstantiateBoss(Scene* scene) override;
    void ConfigureComponents(Boss* boss) override;
    void ConfigureAttacksAndFSM(Boss* boss) override;

    void ConfigureStateOne(Boss *boss, FSMComponent *fsm, ProjectileFactory *spawner) override;
    void ConfigureStateTwo(Boss *boss, FSMComponent *fsm, ProjectileFactory *spawner) override;
    void ConfigureStateThree(Boss *boss, FSMComponent *fsm, ProjectileFactory *spawner) override;
    void ConfigureStateFinal(Boss *boss, FSMComponent *fsm, ProjectileFactory *spawner) override;

};
