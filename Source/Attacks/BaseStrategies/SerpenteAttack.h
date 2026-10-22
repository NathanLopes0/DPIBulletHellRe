//
// A serpente dos sequenciadores: a cabeca persegue, o corpo segue.
//

#pragma once

#include "../IAttackStrategy.h"

/**
 * @class SerpenteAttack
 * @brief Uma serpente de 'numProjectiles' gomos: o primeiro e a cabeca, que
 * persegue o aluno, e os outros passam exatamente por onde ela passou.
 *
 * A IDEIA. A fase final do Luizinho e a dos sequenciadores, e sequi e "seguir":
 * a sequencia e o corpo que vai atras da cabeca, instrucao depois de
 * instrucao. O goto e o que o professor faz com ela - a cabeca salta para um
 * rotulo, e o corpo salta quando chega no ponto do salto.
 *
 * Le de SerpenteAttackParams: espaco, giro, duracao, cabeca, corpo, desvio. De
 * AttackParams: numProjectiles, projectileSpeed, centralAngle (para onde a
 * cabeca sai) e firePosition. angle e IGNORADO.
 */
class SerpenteAttack : public IAttackStrategy {
public:
    SerpenteAttack(ProjectileFactory* spawner, Actor* owner);
    ~SerpenteAttack() override = default;

    std::vector<std::unique_ptr<Projectile>> Execute(const AttackParams& params) override;
};
