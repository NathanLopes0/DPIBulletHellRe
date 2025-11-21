//
// Created by nslop on 05/04/2024.
//

#include <SDL.h>
#include "../Actors/Actor.h"

#include "RigidBodyComponent.h"

const float MAX_SPEED_X = 1000.0f;
const float MAX_SPEED_Y = 1000.0f;
const float GRAVITY = 2000.0f;

RigidBodyComponent::RigidBodyComponent(class Actor* owner, float mass, float friction, int updateOrder)
        :Component(owner, updateOrder)
        ,mMass(mass)
        ,mFrictionCoefficient(friction)
        ,mVelocity(Vector2::Zero)
        ,mAcceleration(Vector2::Zero)
{

}

void RigidBodyComponent::ApplyForce(const Vector2 &force) {
    mAcceleration += force * (1.f/mMass);
}

void RigidBodyComponent::Update(float deltaTime)
{

    //-------MODIfICADO PARA O MOVIMENTO DO JOGO SER COMO DESEJADO-------//


    // Euler Integration (tirei o deltaTime daqui (?))
    mVelocity += mAcceleration * deltaTime;

    mVelocity.x = Math::Clamp<float>(mVelocity.x, -MAX_SPEED_X, MAX_SPEED_X);
    mVelocity.y = Math::Clamp<float>(mVelocity.y, -MAX_SPEED_Y, MAX_SPEED_Y);

    if(Math::NearZero(mVelocity.x, 1.0f)) {
        mVelocity.x = 0.f;
    }

    Vector2 position = mOwner->GetPosition();
    mOwner->SetPosition(position + mVelocity * deltaTime);

    mAcceleration.Set(0.f, 0.f);

}
