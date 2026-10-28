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

    /// O cartao do professor, sem a extensao: o mesmo que ConfigureComponents
    /// desenha, e o que a escolha do orientador mostra.
    [[nodiscard]] std::string Retrato() const override { return "Teachers/DPIBHMariaLucia"; }
protected:
    std::unique_ptr<Boss> InstantiateBoss(Scene* scene) override;
    void ConfigureComponents(Boss* boss) override;
    void ConfigureAttacksAndFSM(Boss* boss) override;
};
