//
// Created by nslop on 23/07/2024.
//

#include "CircleColliderComponent.h"
#include "../../Actors/Actor.h"

CircleColliderComponent::CircleColliderComponent(class Actor *owner, float radius, int updateOrder)
        : ColliderComponent(owner, updateOrder),
        mRadius(radius) {

}

const Vector2& CircleColliderComponent::GetCenter() const {
    return mOwner->GetPosition();
}

float CircleColliderComponent::GetRadius() const {
    return mOwner->GetScale() * mRadius;
}

bool CircleColliderComponent::Intersect(const CircleColliderComponent &b) const {

    // Checa os DOIS colisores. Antes so o 'this' era verificado, entao desligar
    // o colisor de um projetil nao surtia efeito nenhum quando a chamada partia
    // do outro lado - que e exatamente como Battle::CheckCollisions faz
    // (playerCollider->Intersect(*bossProjCollider)).
    if (!mIsEnabled || !b.mIsEnabled) return false;

    Vector2 diff = GetCenter() - b.GetCenter();
    float distSq = diff.LengthSq();

    float radiiSq = GetRadius() + b.GetRadius();
    radiiSq *= radiiSq;

    return distSq <= radiiSq;
}