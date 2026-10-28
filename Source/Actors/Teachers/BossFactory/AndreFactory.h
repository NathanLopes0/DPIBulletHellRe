//
// Created by gensh on 27/11/2025.
//

#pragma once
#include "IBossFactory.h"

class AndreFactory : public IBossFactory {
public:
    explicit AndreFactory(Game* game);
    ~AndreFactory() override = default;

    /// O cartao do professor, sem a extensao: o mesmo que ConfigureComponents
    /// desenha, e o que a escolha do orientador mostra.
    [[nodiscard]] std::string Retrato() const override { return "Teachers/DPIBHAndre"; }

private:

    // Os 3 passos da função principal CreateBoss da classe base
    std::unique_ptr<Boss> InstantiateBoss(Scene* scene) override;
    void ConfigureComponents(Boss* boss) override;
    void ConfigureAttacksAndFSM(Boss* boss) override;

};
