//
// O projetil de chefe, unico tipo concreto.
//

#pragma once

#include "BossProjectile.h"

/// O tipo concreto de todo projetil disparado por um chefe.
class ProjetilDeChefe : public BossProjectile {
public:
    explicit ProjetilDeChefe(Scene* scene, Boss* owner)
        : BossProjectile(scene, owner) {}
};
