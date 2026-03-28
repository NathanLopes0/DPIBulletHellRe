//
// Created by gensh on 26/11/2025.
//

#pragma once
#include "IBossFactory.h"
#include "../../ProjectileFactory.h"


class RicardoFactory : public IBossFactory {
public:
    explicit RicardoFactory(Game* game);
    ~RicardoFactory() override = default;
protected:
    // Os 3 passos do CreateBoss()
    std::unique_ptr<Boss> InstantiateBoss(Scene* scene) override;
    void ConfigureComponents(Boss* boss) override;
    void ConfigureAttacksAndFSM(Boss* boss) override;

    void ConfigureStateOne(Boss *boss, FSMComponent *fsm) override;
    void ConfigureStateTwo(Boss *boss, FSMComponent *fsm) override;
    void ConfigureStateThree(Boss *boss, FSMComponent *fsm) override;
    void ConfigureStateFinal(Boss *boss, FSMComponent *fsm) override;
};
