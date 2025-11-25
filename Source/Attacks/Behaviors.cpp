//
// Created by gensh on 17/10/2025.
//


#include "Behaviors.h"
#include "../Components/RigidBodyComponent.h"
#include "../Actors/Teachers/Bosses/BossesProjectiles/BossProjectile.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"


void HomingBehavior::update(Projectile* p, float deltaTime) {

    if (auto bossProj = dynamic_cast<BossProjectile*>(p)) {
        if(!homing) {
            elapsedTime += deltaTime;
            if(elapsedTime >= homingDelay) {
                homing = true;
                if(homingSpeed == 0) {
                    auto currSpeed = bossProj->GetForwardSpeed();
                    bossProj->GetComponent<RigidBodyComponent>()->SetVelocity(
                                bossProj->GetPlayerDirection() * currSpeed);
                }
                else {
                    bossProj->GetComponent<RigidBodyComponent>()->SetVelocity(
                                bossProj->GetPlayerDirection() * homingSpeed);
                }
            }
        }
    }
}

void AccelerateBehavior::update(Projectile* p, float deltaTime) {
    if (auto bossProj = dynamic_cast<BossProjectile*>(p)) {
        if(!accelerated) {
            elapsedTime += deltaTime;
            if(elapsedTime >= accelerateDelay) {
                accelerated = true;
                auto currVel = bossProj->GetComponent<RigidBodyComponent>()->GetVelocity();
                auto currVelAux = currVel;
                currVel.Normalize();
                if(accelerateSpeedValue != -1) {
                    currVel *= static_cast<float>(accelerateSpeedValue);
                    bossProj->GetComponent<RigidBodyComponent>()->SetVelocity(currVelAux + currVel);
                }
                else {
                    accelerateSpeedPercent = Math::Max(accelerateSpeedPercent, 1.f);
                    bossProj->GetComponent<RigidBodyComponent>()->SetVelocity(currVelAux * accelerateSpeedPercent);
                }
            }
        }
    }
}

void SlowDownBehavior::update(Projectile* p, float deltaTime) {
    if (auto bossProj = dynamic_cast<BossProjectile*>(p)) {
        if(!slowedDown) {
            elapsedTime += deltaTime;
            if(elapsedTime >= slowdownDelay) {
                slowedDown = true;
                auto currVel = p->GetComponent<RigidBodyComponent>()->GetVelocity();
                auto currVelAux = currVel;
                currVel.Normalize();
                if(slowdownSpeedValue != -1) {
                    currVel *= (float) slowdownSpeedValue;
                    bossProj->GetComponent<RigidBodyComponent>()->SetVelocity(currVelAux - currVel);
                }
                else {
                    Math::Clamp(slowdownSpeedPercent, 0.f, 1.f);
                    bossProj->GetComponent<RigidBodyComponent>()->SetVelocity(currVelAux * slowdownSpeedPercent);
                }
            }
        }
    }
}

void ActivateBehavior::update(Projectile* p, float deltaTime) {
    if (auto bossProj = dynamic_cast<BossProjectile*>(p)) {
        if(!activated) {
            elapsedTime += deltaTime;
            if(elapsedTime >= activationDelay) {
                p->GetComponent<DrawAnimatedComponent>()->SetIsVisible(true);
                p->GetComponent<RigidBodyComponent>()->SetVelocity(activationVelocity);
                activated = true;
            }
        }
        else
        {
            p->GetComponent<DrawAnimatedComponent>()->SetIsVisible(false);
        }
    }
}
