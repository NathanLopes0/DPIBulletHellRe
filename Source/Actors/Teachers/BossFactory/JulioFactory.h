//
// Fábrica do Júlio (INF 420)
//

#pragma once

#include "IBossFactory.h"

/**
 * @class JulioFactory
 * @brief Monta o Júlio. As fases dele vivem em Assets/Attacks/fases.json.
 *
 * Progressão temática (um modelo aprendendo a te acertar):
 *   StateOne   "Está te procurando"        um caçador grande que investe em ciclos
 *   StateTwo   "Aprendeu onde você está"   mira exata, projéteis que curvam
 *   StateThree "Aprendeu como você se move" prevê para onde você vai
 *   StateFinal "Entrou em loop"            repescagem: as balas giram antes de vir
 *
 * O que sobrou em C++ é só a mira, em Julio::CustomizeAttackParams, porque ela
 * depende da posição e da velocidade do jogador no instante do disparo.
 */
class JulioFactory : public IBossFactory {
public:
    explicit JulioFactory(Game* game);

protected:
    std::unique_ptr<Boss> InstantiateBoss(Scene* scene) override;
    void ConfigureComponents(Boss* boss) override;
    void ConfigureAttacksAndFSM(Boss* boss) override;
};
