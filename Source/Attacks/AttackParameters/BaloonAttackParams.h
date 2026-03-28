//
// Created by gensh on 20/03/2026.
//

#ifndef DPIBULLETHELLRE_BALOONATTACKPARAMS_H
#define DPIBULLETHELLRE_BALOONATTACKPARAMS_H
#include "AttackParams.h"

struct BaloonAttackParams : AttackParams {

    enum side {
        None,
        Down,
        Left,
        Right,
        Up
    };

    side side = None;
    bool randomSpawn = false;
    bool centerOnPlayer = true;
    float centerOnPlayerOffset = 0.0f;
    std::vector<Vector2> spawnPoints;

};

#endif //DPIBULLETHELLRE_BALOONATTACKPARAMS_H