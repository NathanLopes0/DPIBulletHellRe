//
// Fábrica do Júlio (INF 420)
//

#pragma once

#include "IBossFactory.h"

/**
 * @class JulioFactory
 * @brief Monta o Júlio e suas fases.
 *
 * Progressão temática (um modelo sendo treinado):
 *   StateOne   "Exploração"           busca aleatória, mira ruim, ritmo lento
 *   StateTwo   "Descida do Gradiente" mira exata, projéteis que curvam
 *   StateThree "Overfitting"          leque largo com projéteis serpenteando
 *   StateFinal "Regularização"        repescagem: simples, lento e previsível
 */
class JulioFactory : public IBossFactory {
public:
    explicit JulioFactory(Game* game);

protected:
    std::unique_ptr<Boss> InstantiateBoss(Scene* scene) override;
    void ConfigureComponents(Boss* boss) override;
    void ConfigureAttacksAndFSM(Boss* boss) override;

    void ConfigureStateOne(Boss* boss, FSMComponent* fsm) override;
    void ConfigureStateTwo(Boss* boss, FSMComponent* fsm) override;
    void ConfigureStateThree(Boss* boss, FSMComponent* fsm) override;
    void ConfigureStateFinal(Boss* boss, FSMComponent* fsm) override;
};
