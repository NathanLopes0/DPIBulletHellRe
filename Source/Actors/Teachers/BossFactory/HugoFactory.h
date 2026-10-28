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

    /// O cartao do professor, sem a extensao: o mesmo que ConfigureComponents
    /// desenha, e o que a escolha do orientador mostra.
    [[nodiscard]] std::string Retrato() const override { return "Teachers/DPIBHHugo"; }
protected:
    std::unique_ptr<Boss> InstantiateBoss(Scene* scene) override;
    void ConfigureComponents(Boss* boss) override;
    void ConfigureAttacksAndFSM(Boss* boss) override;
};
