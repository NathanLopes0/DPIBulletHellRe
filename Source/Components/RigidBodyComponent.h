//
// Created by nslop on 05/04/2024.
//

#pragma once

#include "Component.h"
#include "../Motor/Math.h"

class RigidBodyComponent : public Component {

public:
    // Lower update order to update first.
    RigidBodyComponent(class Actor* owner, float mass = 1.0f, float friction = 0.0f, int updateOrder = 10);

    void Update(float deltaTime) override;

    const Vector2& GetVelocity() const { return mVelocity; }
    void SetVelocity(const Vector2& velocity) { mVelocity = velocity; }

    void SetAcceleration(const Vector2& acceleration) { mAcceleration = acceleration; }

    /// O teto da velocidade, em cada eixo. O padrao vale para todo mundo; o tiro do aluno, mais
    /// rapido que isso, pede o dele.
    static constexpr float kVelocidadeMaximaPadrao = 1000.0f;
    void DefinirVelocidadeMaxima(const float porEixo) { mVelocidadeMaxima = porEixo; }

private:
    // Physical properties.
    float mFrictionCoefficient;
    float mMass;
    float m;

    Vector2 mVelocity;
    Vector2 mAcceleration;
    float mVelocidadeMaxima = kVelocidadeMaximaPadrao;
};
