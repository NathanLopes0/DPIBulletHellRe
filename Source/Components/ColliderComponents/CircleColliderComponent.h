//
// Created by nslop on 23/07/2024.
//

#ifndef DPIBULLETHELLRE_CIRCLECOLLIDERCOMPONENT_H
#define DPIBULLETHELLRE_CIRCLECOLLIDERCOMPONENT_H

#include "ColliderComponent.h"
#include "../Component.h"
#include "../../Math.h"


class CircleColliderComponent : public ColliderComponent {

public:
    CircleColliderComponent(Actor* owner, float radius, int updateOrder = 10);

    void SetRadius(const float radius) { mRadius = radius; }
    [[nodiscard]] float GetRadius() const;

    [[nodiscard]] const Vector2& GetCenter() const;
    [[nodiscard]] bool Intersect(const CircleColliderComponent& b) const;

private:
    float mRadius;
};


#endif //DPIBULLETHELLRE_CIRCLECOLLIDERCOMPONENT_H
