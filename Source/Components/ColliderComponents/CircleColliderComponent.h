//
// Created by nslop on 23/07/2024.
//

#ifndef DPIBULLETHELLRE_CIRCLECOLLIDERCOMPONENT_H
#define DPIBULLETHELLRE_CIRCLECOLLIDERCOMPONENT_H

#include "../Component.h"
#include "../../Math.h"


class CircleColliderComponent : public Component {

public:
    CircleColliderComponent(class Actor* owner, float radius, int updateOrder = 10);

    void SetRadius(float radius) { mRadius = radius; }
    float GetRadius() const;

    const Vector2& GetCenter() const;
    bool Intersect(const CircleColliderComponent& b) const;

private:
    float mRadius;
};


#endif //DPIBULLETHELLRE_CIRCLECOLLIDERCOMPONENT_H
