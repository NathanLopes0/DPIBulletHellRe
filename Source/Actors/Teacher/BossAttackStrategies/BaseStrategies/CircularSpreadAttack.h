//
// Created by nslop on 26/11/2024.
//

#ifndef DPIBULLETHELLRE_CIRCULARSPREADATTACK_H
#define DPIBULLETHELLRE_CIRCULARSPREADATTACK_H


#include "../../../../AttackStrategy.h"

class CircularSpreadAttack : public AttackStrategy {
public:
    CircularSpreadAttack(class ProjectileFactory* spawner, class Boss* owner, int _numProjectiles, float _projectileSpeed);
    ~CircularSpreadAttack();

    std::vector<class Projectile*> execute() override;


private:
    class ProjectileFactory* mPSpawner;
    class Boss* mOwner;
    int numProjectiles;
    float projectileSpeed;


    //DEBUG
    bool mAttacked = false;

};


#endif //DPIBULLETHELLRE_CIRCULARSPREADATTACK_H
