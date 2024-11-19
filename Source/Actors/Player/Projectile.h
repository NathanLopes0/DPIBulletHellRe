//
// Created by nslop on 18/11/2024.
//

#ifndef DPIBULLETHELLRE_PROJECTILE_H
#define DPIBULLETHELLRE_PROJECTILE_H

#include "../Actor.h"

class Projectile : public Actor {

public:
    Projectile(class Scene* scene);
    void OnUpdate(float deltaTime) override;
    void OnCollision() override;

private:

    class DrawAnimatedComponent* mDrawComponent;
    class RigidBodyComponent* mRigidBodyComponent;
    class CircleColliderComponent* mColliderComponent;



    //subfunções
    bool InsideProjectileLimit();
    void DeactivateProjectile();
};


#endif //DPIBULLETHELLRE_PROJECTILE_H
