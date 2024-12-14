//
// Created by nslop on 18/11/2024.
//

#ifndef DPIBULLETHELLRE_PROJECTILE_H
#define DPIBULLETHELLRE_PROJECTILE_H

#include "Actor.h"
#include <memory>
#include "../Components/RigidBodyComponent.h"
#include "Teacher/BossAttackStrategies/Behaviors.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Components/ColliderComponents/CircleColliderComponent.h"


class Projectile : public Actor {

public:
    explicit Projectile(class Scene* scene);
    void OnUpdate(float deltaTime) override;
    void OnCollision() override;

    virtual void ActivateProjectile(const Vector2& desiredStartingPosition);
    virtual void DeactivateProjectile() = 0;

    virtual ~Projectile() = default;


    float GetFowardSpeed();


    template <typename Behavior, typename... Args>
    void insertBehavior(Args&&... args) {
        mBehaviors.push_back(std::make_unique<Behavior>(std::forward<Args>(args)...));
    }

protected:

    class DrawAnimatedComponent* mDrawComponent;
    class RigidBodyComponent* mRigidBodyComponent;
    class CircleColliderComponent* mColliderComponent;

    //Behavior structures and methods
    std::vector<std::unique_ptr<class ProjectileBehavior>> mBehaviors;


    //Sub-funções de OnUpdate
    [[nodiscard]] virtual bool InsideProjectileLimit() const = 0;

};


#endif //DPIBULLETHELLRE_PROJECTILE_H
