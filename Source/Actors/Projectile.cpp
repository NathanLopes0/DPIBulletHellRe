//
// Created by nslop on 18/11/2024.
//

#include "Projectile.h"
#include "../Scenes/Scene.h"
#include "../Components/Component.h"


Projectile::Projectile(Scene *scene, Actor* owner) :
        Actor(scene),
        mOwner(owner),
        mForwardSpeed(0.0f)
{

}

void Projectile::OnUpdate(float deltaTime) {

    if (IsOffScreen()) {
        SetState(ActorState::Destroy);
    }

}

void Projectile::OnCollision(Actor* other) {


}
