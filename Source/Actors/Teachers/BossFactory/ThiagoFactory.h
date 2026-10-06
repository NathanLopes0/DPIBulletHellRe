//
// Fabrica do Thiago (INF 220)
//

#pragma once

#include "IBossFactory.h"

class ThiagoFactory : public IBossFactory {
public:
    explicit ThiagoFactory(Game* game);

protected:
    std::unique_ptr<Boss> InstantiateBoss(Scene* scene) override;
    void ConfigureComponents(Boss* boss) override;
    void ConfigureAttacksAndFSM(Boss* boss) override;
};
