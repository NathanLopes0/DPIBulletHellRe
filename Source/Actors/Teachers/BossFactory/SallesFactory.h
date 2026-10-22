#pragma once
#include "IBossFactory.h"
#include "../../ProjectileFactory.h"


class SallesFactory : public IBossFactory {
public:
    explicit SallesFactory(Game* game);
    ~SallesFactory() override = default;

    /// O cartao do professor, sem a extensao: o mesmo que ConfigureComponents
    /// desenha, e o que a escolha do orientador mostra.
    [[nodiscard]] std::string Retrato() const override { return "Teachers/DPIBHSalles"; }

protected:
    // Os 3 passos que o CreateBoss() vai chamar
    std::unique_ptr<Boss> InstantiateBoss(Scene* scene) override;
    void ConfigureComponents(Boss* boss) override;
    void ConfigureAttacksAndFSM(Boss* boss) override;
};
