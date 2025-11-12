#pragma once
#include "IBossFactory.h"


class SallesFactory : public IBossFactory {
public:
    explicit SallesFactory(Game* game);
    ~SallesFactory() override = default;

protected:
    // Os 3 passos que o CreateBoss() vai chamar
    std::unique_ptr<Boss> InstantiateBoss(Scene* scene) override;
    void ConfigureComponents(Boss* boss) override;
    void ConfigureAttacksAndFSM(Boss* boss) override;

    void ConfigureStateOne(Boss *boss, FSMComponent *fsm, ProjectileFactory *spawner) override;
    void ConfigureStateTwo(Boss *boss, FSMComponent *fsm, ProjectileFactory *spawner) override;
};