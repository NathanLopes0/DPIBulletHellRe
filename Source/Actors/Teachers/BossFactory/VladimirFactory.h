//
// A fabrica do chefe da INF 331.
//

#pragma once
#include "IBossFactory.h"
#include "../../ProjectileFactory.h"

class VladimirFactory : public IBossFactory {
public:
    explicit VladimirFactory(Game* game);
    ~VladimirFactory() override = default;
protected:
    // Os 3 passos do CreateBoss()
    std::unique_ptr<Boss> InstantiateBoss(Scene* scene) override;
    void ConfigureComponents(Boss* boss) override;
    void ConfigureAttacksAndFSM(Boss* boss) override;
};
