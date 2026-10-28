//
// A sprint dos metodos ageis: o cursor arrasta as historias para o quadro.
//

#pragma once

#include "../IAttackStrategy.h"

/**
 * @class SprintAttack
 * @brief Uma sprint de 'numProjectiles' historias, arrastadas por um cursor do
 * professor ate o quadro, onde esperam a revisao.
 *
 * A IDEIA. A fase 2 da Maria Lucia e a dos metodos ageis, e cada etapa dela e
 * uma sprint. O cursor - a IHC - arrasta a pilha de historias e a solta no
 * quadro; o aluno ENTREGA uma historia derrubando-a a tiro (o tipo do projetil
 * e "derrubavel"), e na revisao o que sobrou cai em cima dele, na velocidade
 * do ataque. As contas estao em Sprint.h.
 *
 * Na mao do cursor a pilha e BLINDADA, e o tiro some nela. Parada na vaga, a
 * historia esta EM ANDAMENTO: entra no posto do quadro, e uma PontosAttack da
 * mesma fase atira de cada historia parada.
 *
 * O CURSOR E UM ENFEITE (BossProjectile::DefinirEnfeite): sai da mesma fabrica
 * que as historias, para usar a mesma folha, mas nao acerta o aluno e o tiro
 * passa por ele. E a mao do professor, e nao um projetil.
 *
 * Le de SprintAttackParams: revisao, historia, cursor, clique. De
 * AttackParams: numProjectiles, projectileSpeed (a queda) e firePosition.
 * angle e centralAngle sao IGNORADOS.
 */
class SprintAttack : public IAttackStrategy {
public:
    SprintAttack(ProjectileFactory* spawner, Actor* owner);
    ~SprintAttack() override = default;

    std::vector<std::unique_ptr<Projectile>> Execute(const AttackParams& params) override;
};
