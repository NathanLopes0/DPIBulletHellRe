//
// A fabrica da chefe da INF 221.
//

#pragma once
#include "IBossFactory.h"
#include "../../ProjectileFactory.h"

class MariaLuciaFactory : public IBossFactory {
public:
    explicit MariaLuciaFactory(Game* game);
    ~MariaLuciaFactory() override = default;
protected:
    std::unique_ptr<Boss> InstantiateBoss(Scene* scene) override;
    void ConfigureComponents(Boss* boss) override;
    void ConfigureAttacksAndFSM(Boss* boss) override;
};
