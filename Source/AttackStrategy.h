//
// Created by nslop on 23/11/2024.
//

#ifndef DPIBULLETHELLRE_ATTACKSTRATEGY_H
#define DPIBULLETHELLRE_ATTACKSTRATEGY_H

#include <vector>

class AttackStrategy {

public:
    virtual ~AttackStrategy() = default;

    // GAMBIARRA TOTAL pra fazer Angled poder mudar os parametros no meio da execução, deve ter como fazer com Template
    // Mas por enquanto ta funcionando.
    virtual std::vector<class Projectile*> execute() = 0;
    virtual std::vector<class Projectile*> execute(float) { return {}; };
    virtual std::vector<class Projectile*> execute(float, float) { return {}; }

    //AngledAttack
    virtual std::vector<class Projectile*> execute(float, float, float) { return {}; }

    //AngledFilledAttack
    virtual std::vector<class Projectile*> execute(int, float, float, float) { return {}; }
    virtual std::vector<class Projectile*> execute(float, float, float, float) { return {}; }

    //WaveAttack
    virtual std::vector<class Projectile*> execute(int, float, float, float, float) { return {}; }


};


#endif //DPIBULLETHELLRE_ATTACKSTRATEGY_H
