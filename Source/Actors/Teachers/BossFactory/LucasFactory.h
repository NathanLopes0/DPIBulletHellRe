//
// A fabrica do chefe da INF 321.
//

#pragma once
#include "IBossFactory.h"
#include "../../ProjectileFactory.h"

class LucasFactory : public IBossFactory {
public:
    explicit LucasFactory(Game* game);
    ~LucasFactory() override = default;
protected:
    // Os 3 passos do CreateBoss()
    std::unique_ptr<Boss> InstantiateBoss(Scene* scene) override;
    void ConfigureComponents(Boss* boss) override;
    void ConfigureAttacksAndFSM(Boss* boss) override;
};
