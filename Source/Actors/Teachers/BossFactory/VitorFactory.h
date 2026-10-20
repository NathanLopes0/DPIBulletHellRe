//
// A fabrica do chefe da INF 310.
//

#pragma once
#include "IBossFactory.h"
#include "../../ProjectileFactory.h"

class VitorFactory : public IBossFactory {
public:
    explicit VitorFactory(Game* game);
    ~VitorFactory() override = default;
protected:
    std::unique_ptr<Boss> InstantiateBoss(Scene* scene) override;
    void ConfigureComponents(Boss* boss) override;
    void ConfigureAttacksAndFSM(Boss* boss) override;
};
