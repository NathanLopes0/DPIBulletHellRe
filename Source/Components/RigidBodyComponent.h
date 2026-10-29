//
// Created by nslop on 05/04/2024.
//

#pragma once

#include "Component.h"
#include "../Core/Math.h"

class RigidBodyComponent : public Component {

public:
    // Lower update order to update first
    RigidBodyComponent(class Actor* owner, float mass = 1.0f, float friction = 0.0f, int updateOrder = 10);

    void Update(float deltaTime) override;

    const Vector2& GetVelocity() const { return mVelocity; }
    void SetVelocity(const Vector2& velocity) { mVelocity = velocity; }

    void SetAcceleration(const Vector2& acceleration) { mAcceleration = acceleration; }

private:
    // Physical properties
    float mFrictionCoefficient;
    float mMass;
    float m;

    Vector2 mVelocity;
    Vector2 mAcceleration;
};
