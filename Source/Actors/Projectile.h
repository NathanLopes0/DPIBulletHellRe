//
// Created by nslop on 18/11/2024.
//

#ifndef DPIBULLETHELLRE_PROJECTILE_H
#define DPIBULLETHELLRE_PROJECTILE_H

#include "Actor.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Components/ColliderComponents/CircleColliderComponent.h"


class Projectile : public Actor {

public:
    explicit Projectile(class Scene* scene);
    void OnUpdate(float deltaTime) override;
    void OnCollision() override;

    virtual void ActivateProjectile(const Vector2& desiredStartingPosition);
    virtual void DeactivateProjectile() = 0;


    void SetFowardSpeed(float newSpeed) { mFowardSpeed = newSpeed; }


protected:

    class DrawAnimatedComponent* mDrawComponent;
    class RigidBodyComponent* mRigidBodyComponent;
    class CircleColliderComponent* mColliderComponent;
    class Actor* mOwner;
    float mFowardSpeed;


    //Sub-funções de OnUpdate
    [[nodiscard]] virtual bool InsideProjectileLimit() const = 0;

};


#endif //DPIBULLETHELLRE_PROJECTILE_H
