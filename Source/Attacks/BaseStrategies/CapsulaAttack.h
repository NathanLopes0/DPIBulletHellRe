//
// A capsula do encapsulamento: as letras viajam fechadas e abrem num anel.
//

#pragma once

#include "../IAttackStrategy.h"

/**
 * @class CapsulaAttack
 * @brief As letras saem EMPILHADAS, como uma capsula so, e abrem num anel.
 *
 * A IDEIA. A fase 2 do Luizinho e o encapsulamento - in capsula, "na
 * caixinha". O que esta dentro da capsula nao se ve e nao se alcanca de fora:
 *
 *   - as 'numProjectiles' letras nascem no mesmo ponto, com a mesma
 *     velocidade, e com a cara da casca (a regra da rajada da a animacao), de
 *     modo que a pilha inteira se le como UMA capsula;
 *   - fechada ela e BLINDADA (BossProjectile::Blindado): o tiro do aluno some
 *     nela e nao chega ao professor;
 *   - ela so abre por dentro, aos 'abre' segundos de voo, num anel
 *     (Capsula::Abertura), e cada letra troca para a animacao 'escreve' e
 *     volta ao tamanho de fabrica - a casca pode ser maior que o conteudo.
 *
 * Le de CapsulaAttackParams: abre, velocidadeDaAbertura, escreve. De
 * AttackParams: numProjectiles (quantas letras dentro), projectileSpeed (o voo
 * da capsula), centralAngle (para onde ela voa) e firePosition. angle e
 * IGNORADO.
 */
class CapsulaAttack : public IAttackStrategy {
public:
    CapsulaAttack(ProjectileFactory* spawner, Actor* owner);
    ~CapsulaAttack() override = default;

    std::vector<std::unique_ptr<Projectile>> Execute(const AttackParams& params) override;
};
