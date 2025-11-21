//
// Created by gensh on 01/11/2025.
//

#ifndef DPIBULLETHELLRE_COLLIDERCOMPONENT_H
#define DPIBULLETHELLRE_COLLIDERCOMPONENT_H


#include "../Component.h"

enum class ColliderTag {
    None,
    Player,
    PlayerProjectile,
    Boss,
    BossProjectile
};

class ColliderComponent : public Component {

public:
    explicit ColliderComponent(Actor* owner, int updateOrder = 100);
    void SetTag(const ColliderTag tag) { mTag = tag; }
    [[nodiscard]] ColliderTag GetColliderTag() const { return mTag; }

protected:
    ColliderTag mTag = ColliderTag::None;
};


#endif //DPIBULLETHELLRE_COLLIDERCOMPONENT_H