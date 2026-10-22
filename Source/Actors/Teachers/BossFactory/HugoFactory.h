//
// A fabrica do chefe do TCC.
//

#pragma once
#include "IBossFactory.h"
#include "../../ProjectileFactory.h"

class HugoFactory : public IBossFactory {
public:
    explicit HugoFactory(Game* game);
    ~HugoFactory() override = default;
protected:
    std::unique_ptr<Boss> InstantiateBoss(Scene* scene) override;
    void ConfigureComponents(Boss* boss) override;
    void ConfigureAttacksAndFSM(Boss* boss) override;
};
