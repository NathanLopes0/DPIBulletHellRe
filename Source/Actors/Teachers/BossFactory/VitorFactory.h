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

    /// O cartao do professor, sem a extensao: o mesmo que ConfigureComponents
    /// desenha, e o que a escolha do orientador mostra.
    [[nodiscard]] std::string Retrato() const override { return "Teachers/DPIBHVitor"; }
protected:
    std::unique_ptr<Boss> InstantiateBoss(Scene* scene) override;
    void ConfigureComponents(Boss* boss) override;
    void ConfigureAttacksAndFSM(Boss* boss) override;
};
