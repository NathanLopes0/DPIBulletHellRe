//
// Modified by nslop on 17/10/2025.
//
//
// Created by nslop on 26/11/2024.
//

#ifndef DPIBULLETHELLRE_CIRCULARSPREADATTACK_H
#define DPIBULLETHELLRE_CIRCULARSPREADATTACK_H

#include <vector>
#include "../IAttackStrategy.h"

class Projectile;

class CircularSpreadAttack : public IAttackStrategy {
public:
    CircularSpreadAttack(class ProjectileFactory* spawner, class Boss* owner, int _numProjectiles, float _projectileSpeed);
    ~CircularSpreadAttack() override = default;

    std::vector<Projectile*> Execute();



private:
    class ProjectileFactory* mPSpawner;
    class Boss* mOwner;
    int numProjectiles;
    float projectileSpeed;


    //DEBUG
    bool mAttacked = false;

};


#endif //DPIBULLETHELLRE_CIRCULARSPREADATTACK_H