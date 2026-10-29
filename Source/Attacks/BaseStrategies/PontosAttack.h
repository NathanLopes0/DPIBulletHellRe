//
// O disparo que sai de varios pontos do campo ao mesmo tempo.
//

#pragma once

#include "../Engine/IAttackStrategy.h"

/**
 * @class PontosAttack
 * @brief Um leque de 'numProjectiles' de CADA ponto de um posto, e nao do
 * chefe.
 *
 * A IDEIA. Ha coisas do professor em campo que atiram por ele: as historias
 * em andamento no quadro da sprint da Maria Lucia (Sprint.h), e - nos chefes
 * que vem depois - os robos do Julio e as nuvens do Vitor. A estrategia nao
 * sabe o que sao: quem diz onde estao e o Boss (PontosDoPosto), na hora do
 * disparo, e um posto vazio nao dispara nada.
 *
 * Le de PontosAttackParams: pontos, mirar, prever, alvo. De AttackParams:
 * numProjectiles e angle (o leque de cada ponto), projectileSpeed, e
 * centralAngle quando nao mira. firePosition e IGNORADO.
 */
class PontosAttack : public IAttackStrategy {
public:
    PontosAttack(ProjectileFactory* spawner, Actor* owner);
    ~PontosAttack() override = default;

    std::vector<std::unique_ptr<Projectile>> Execute(const AttackParams& params) override;
};
