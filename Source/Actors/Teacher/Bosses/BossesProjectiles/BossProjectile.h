//
// Created by nslop on 21/11/2024.
//

#ifndef DPIBULLETHELLRE_BOSSPROJECTILE_H
#define DPIBULLETHELLRE_BOSSPROJECTILE_H

#include "../../../Projectile.h"


class BossProjectile : public Projectile {

public:

    //TODO 23.2 - Dar um jeito de Boss* owner ser aceito como mOwner de Projectile.cpp também (o Owner do Projectile é Actor).

    BossProjectile(class Scene* scene, class Boss* owner);
    void OnUpdate(float deltaTime) override;


    void DeactivateProjectile() override;

    void insertComponents(DrawAnimatedComponent *pComponent, CircleColliderComponent *pComponent1, RigidBodyComponent* pComponent2);
    const Vector2& GetPlayerDirection();
protected:

    [[nodiscard]] bool InsideProjectileLimit() const override;
    bool CheckCollision();


private:
    const Vector2& GetPlayerPosition();
    class Boss* mOwner;
};


#endif //DPIBULLETHELLRE_BOSSPROJECTILE_H
