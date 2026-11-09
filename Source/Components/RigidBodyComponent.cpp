//
// Created by nslop on 05/04/2024.
//

#include "../Actors/Actor.h"

#include "RigidBodyComponent.h"

RigidBodyComponent::RigidBodyComponent(class Actor* owner, float mass, float friction, int updateOrder)
        :Component(owner, updateOrder)
        ,mMass(mass)
        ,mFrictionCoefficient(friction)
        ,mVelocity(Vector2::Zero)
        ,mAcceleration(Vector2::Zero)
{

}

void RigidBodyComponent::Update(float deltaTime)
{

    // ---- MODIfICADO para o movimento do jogo ser como desejado-------// ----

    // Euler Integration.
    mVelocity += mAcceleration * deltaTime;

    mVelocity.x = Math::Clamp<float>(mVelocity.x, -mVelocidadeMaxima, mVelocidadeMaxima);
    mVelocity.y = Math::Clamp<float>(mVelocity.y, -mVelocidadeMaxima, mVelocidadeMaxima);

    if(Math::NearZero(mVelocity.x, 1.0f)) {
        mVelocity.x = 0.f;
    }

    Vector2 position = mOwner->GetPosition();
    mOwner->SetPosition(position + mVelocity * deltaTime);

    mAcceleration.Set(0.f, 0.f);

}

// TimeToHeight = Height/GetVelocity()
