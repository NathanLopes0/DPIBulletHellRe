//
// Created by nslop on 21/11/2024.
//

#ifndef DPIBULLETHELLRE_SALLESBOSSPROJECTILE_H
#define DPIBULLETHELLRE_SALLESBOSSPROJECTILE_H

#include "BossProjectile.h"

class SallesBossProjectile : public BossProjectile {

public:
    SallesBossProjectile(class Scene* scene, class Salles* owner, const std::string& spritePath, const std::string& dataPath);

};


#endif //DPIBULLETHELLRE_SALLESBOSSPROJECTILE_H
