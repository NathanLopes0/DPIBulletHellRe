//
// Fabrica do Thiago (INF 220)
//

#pragma once

#include "IBossFactory.h"

class ThiagoFactory : public IBossFactory {
public:
    explicit ThiagoFactory(Game* game);

    /// O cartao do professor, sem a extensao: o mesmo que ConfigureComponents
    /// desenha, e o que a escolha do orientador mostra.
    [[nodiscard]] std::string Retrato() const override { return "Teachers/DPIBHThiago"; }

protected:
    std::unique_ptr<Boss> InstantiateBoss(Scene* scene) override;
    void ConfigureComponents(Boss* boss) override;
    void ConfigureAttacksAndFSM(Boss* boss) override;
};
