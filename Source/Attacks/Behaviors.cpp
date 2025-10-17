//
// Created by gensh on 17/10/2025.
//


#include "Behaviors.h"
#include "../Components/RigidBodyComponent.h"
#include "../Actors/Teacher/Bosses/BossesProjectiles/BossProjectile.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"

void HomingBehavior::update(BossProjectile* p, float deltaTime) {
    if(!homing) {
        elapsedTime += deltaTime;
        if(elapsedTime >= homingDelay) {
            homing = true;
            if(homingSpeed == 0) {
                auto currSpeed = p->GetForwardSpeed();
                p->GetComponent<RigidBodyComponent>()->SetVelocity(p->GetPlayerDirection() * currSpeed);
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

void ActivateBehavior::update(BossProjectile* p, float deltaTime) {
    if(!activated) {
        elapsedTime += deltaTime;
        if(elapsedTime >= activationDelay) {
            p->GetComponent<DrawAnimatedComponent>()->SetIsVisible(true);
            p->GetComponent<RigidBodyComponent>()->SetVelocity(activationVelocity);
            activated = true;
        }
    }
}
