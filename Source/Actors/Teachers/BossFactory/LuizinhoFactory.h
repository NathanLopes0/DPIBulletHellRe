//
// A fabrica do chefe da INF 340.
//

#pragma once
#include "IBossFactory.h"
#include "../../ProjectileFactory.h"

class LuizinhoFactory : public IBossFactory {
public:
    explicit LuizinhoFactory(Game* game);
    ~LuizinhoFactory() override = default;
protected:
    // Os 3 passos do CreateBoss()
    std::unique_ptr<Boss> InstantiateBoss(Scene* scene) override;
    void ConfigureComponents(Boss* boss) override;
    void ConfigureAttacksAndFSM(Boss* boss) override;
};
