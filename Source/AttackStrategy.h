//
// Created by nslop on 23/11/2024.
//

#ifndef DPIBULLETHELLRE_ATTACKSTRATEGY_H
#define DPIBULLETHELLRE_ATTACKSTRATEGY_H

#include <vector>

class AttackStrategy {

    virtual ~AttackStrategy() = default;
    virtual std::vector<class Projectile*> execute() = 0;

};

// TODO 1.0 - A ideia é criar estratégias gerais ("CircleAttack, AngledAttack, SpiralAttack, BurstAttack, WaveAttack)
// TODO 1.0 - e ai usar Decorator para adicionar funcionalidades aos ataques ("Homing", "Accelerate", "Decelerate", "Split", "Orbit")

// TODO 1.0.1 - Esses Decorators todos receberão um vector de Projectiles para manipulá-los e retorná-los.
// TODO 1.0.2 - As estratégias não receberão, mas criarão os vetores e os retornarão.

// TODO 1.0.3 - Acho que a ideia é criar o vetor nas Strategies, criar os projéteis, enviar o vetor para Decorator,
// TODO 1.0.3 - receber o vetor modificado pelo decorator, fazer algo se necessário, e ai finalmente retorná-lo para o Boss.




#endif //DPIBULLETHELLRE_ATTACKSTRATEGY_H
