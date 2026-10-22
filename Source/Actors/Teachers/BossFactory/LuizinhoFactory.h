//
// A fabrica do chefe da INF 340.
//

#pragma once
#include "IBossFactory.h"
#include "../../ProjectileFactory.h"

class LuizinhoFactory : public IBossFactory {
public:
    explicit LuizinhoFactory(Game* game);
    ~LuizinhoFactory() override = default;

    /// O cartao do professor, sem a extensao: o mesmo que ConfigureComponents
    /// desenha, e o que a escolha do orientador mostra.
    [[nodiscard]] std::string Retrato() const override { return "Teachers/DPIBHLuizinho"; }
protected:
    // Os 3 passos do CreateBoss()
    std::unique_ptr<Boss> InstantiateBoss(Scene* scene) override;
    void ConfigureComponents(Boss* boss) override;
    void ConfigureAttacksAndFSM(Boss* boss) override;
};
