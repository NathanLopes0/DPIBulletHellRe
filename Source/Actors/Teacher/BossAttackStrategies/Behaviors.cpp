//
// Created by nslop on 14/12/2024.
//

#include "Behaviors.h"
#include "../../Teacher/Bosses/BossesProjectiles/BossProjectile.h"


void HomingBehavior::update(BossProjectile* p, float deltaTime) {
    if(!homing) {
        elapsedTime += deltaTime;
        if(elapsedTime >= homingDelay) {
            homing = true;
            auto newSpeed = (homingSpeed == 0) ? 1 : homingSpeed;
            if(newSpeed == 1) {
                p->GetComponent<RigidBodyComponent>()->SetVelocity(p->GetPlayerDirection() * p->GetFowardSpeed());
            }
            else {
                p->GetComponent<RigidBodyComponent>()->SetVelocity(p->GetPlayerDirection() * homingSpeed);
            }
        }
    }
}

void AccelerateBehavior::update(BossProjectile* p, float deltaTime) {
    if(!accelerated) {
        elapsedTime += deltaTime;
        if(elapsedTime >= accelerateDelay) {
            accelerated = true;
            auto currVel = p->GetComponent<RigidBodyComponent>()->GetVelocity();
            auto currVelAux = currVel;
            currVel.Normalize();
            if(accelerateSpeedValue != -1) {
                currVel *= (float)accelerateSpeedValue;
                p->GetComponent<RigidBodyComponent>()->SetVelocity(currVelAux + currVel);
            }
            else {
                accelerateSpeedPercent = Math::Max(accelerateSpeedPercent, 1.f);
                p->GetComponent<RigidBodyComponent>()->SetVelocity(currVelAux * accelerateSpeedPercent);
            }
        }
    }
}

void SlowDownBehavior::update(BossProjectile* p, float deltaTime) {
    if(!slowedDown) {
        elapsedTime += deltaTime;
        if(elapsedTime >= slowdownDelay) {
            slowedDown = true;
            auto currVel = p->GetComponent<RigidBodyComponent>()->GetVelocity();
            auto currVelAux = currVel;
            currVel.Normalize();
            if(slowdownSpeedValue != -1) {
                currVel *= (float) slowdownSpeedValue;
                p->GetComponent<RigidBodyComponent>()->SetVelocity(currVelAux - currVel);
            }
            else {
                Math::Clamp(slowdownSpeedPercent, 0.f, 1.f);
                p->GetComponent<RigidBodyComponent>()->SetVelocity(currVelAux * slowdownSpeedPercent);
            }
        }
    }
}
