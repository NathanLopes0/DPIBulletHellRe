//
// Created by gensh on 26/11/2025.
//

#pragma once
#include "IBossFactory.h"
#include "../../ProjectileFactory.h"


class RicardoFactory : public IBossFactory {
public:
    explicit RicardoFactory(Game* game);
    ~RicardoFactory() override = default;

    /// O cartao do professor, sem a extensao: o mesmo que ConfigureComponents
    /// desenha, e o que a escolha do orientador mostra.
    [[nodiscard]] std::string Retrato() const override { return "Teachers/DPIBHRicardo"; }
protected:
    // Os 3 passos do CreateBoss()
    std::unique_ptr<Boss> InstantiateBoss(Scene* scene) override;
    void ConfigureComponents(Boss* boss) override;
    void ConfigureAttacksAndFSM(Boss* boss) override;
};
