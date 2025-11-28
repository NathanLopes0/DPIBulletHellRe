//
// Created by gensh on 27/11/2025.
//

#include "AndreBossProjectile.h"
#include "../../../../Random.h"
#include "../../../../Components/DrawComponents/DrawAnimatedComponent.h"

AndreBossProjectile::AndreBossProjectile(Scene *scene, Boss *owner) : BossProjectile(scene, owner) {

    const int choose = Random::GetIntRange(0,2);

}
