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

    /// O cartao do professor, sem a extensao: o mesmo que ConfigureComponents
    /// desenha, e o que a escolha do orientador mostra.
    [[nodiscard]] std::string Retrato() const override { return "Teachers/DPIBHVladimir"; }
protected:
    // Os 3 passos do CreateBoss()
    std::unique_ptr<Boss> InstantiateBoss(Scene* scene) override;
    void ConfigureComponents(Boss* boss) override;
    void ConfigureAttacksAndFSM(Boss* boss) override;
};
