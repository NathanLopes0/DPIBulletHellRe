//
// Created by nslop on 25/09/2024.
//

#include "Salles.h"

#include <complex>

#include "../../../Scenes/Battle/Battle.h"
#include "../../Projectile.h"
#include "../../../Random.h"
#include "../../../Attacks/Behaviors.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Scenes/Battle/ProjectileManager.h"
#include "BossesProjectiles/BossProjectile.h"
#include "BossesProjectiles/SallesBossProjectile.h"

Salles::Salles(Scene *scene) : Boss(scene)
{

}

void Salles::OnUpdate(float deltaTime) {

    mAttackCooldown -= deltaTime;
    if (mAttackCooldown <= 0.0f) {
        Attack();
    }
}



