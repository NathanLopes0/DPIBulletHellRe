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

    Vector2 diff = GetCenter() - b.GetCenter();
    float distSq = diff.LengthSq();

    float radiiSq = GetRadius() + b.GetRadius();
    radiiSq *= radiiSq;

    return distSq <= radiiSq;
}