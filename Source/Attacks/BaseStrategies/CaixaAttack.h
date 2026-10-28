//
// A caixa do HTML: um retangulo de projeteis que nasce em volta do chefe e
// cresce sem perder a forma.
//

#pragma once

#include "../IAttackStrategy.h"

/**
 * @class CaixaAttack
 * @brief Um retangulo de projeteis em volta do chefe, que cresce como uma div.
 *
 * A IDEIA. A fase 1 do Lucas e o HTML, e o HTML e caixas dentro de caixas. Cada
 * disparo e uma div: nasce fechada em volta dele e cresce. Disparos seguidos
 * ficam uns dentro dos outros, e o espaco entre duas caixas e o padding.
 *
 * COMO ELA CRESCE. Cada projetil sai na direcao em que esta do centro, com
 * velocidade proporcional a distancia (Caixa::Velocidade). E uma homotetia: a
 * caixa fica semelhante a si mesma o tempo todo, e por isso os ladrilhos se
 * afastam enquanto ela cresce. Ela nasce fechada e chega ao aluno com frestas.
 *
 * DIFERENTE DE UM ANEL de dois jeitos que se sentem jogando: a base chega
 * INTEIRA de uma vez, como uma linha, em vez de pelo ponto mais perto; e os
 * cantos andam mais rapido que o meio dos lados.
 *
 * Le de CaixaAttackParams: proporcao e meiaAlturaInicial. De AttackParams:
 * numProjectiles (quantos no contorno), projectileSpeed (a velocidade da
 * meia-altura) e firePosition. angle e centralAngle sao IGNORADOS.
 */
class CaixaAttack : public IAttackStrategy {
public:
    CaixaAttack(ProjectileFactory* spawner, Actor* owner);
    ~CaixaAttack() override = default;

    std::vector<std::unique_ptr<Projectile>> Execute(const AttackParams& params) override;
};
