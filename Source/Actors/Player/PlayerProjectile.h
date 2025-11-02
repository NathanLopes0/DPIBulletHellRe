//
// Created by nslop on 19/11/2024.
//

#ifndef DPIBULLETHELLRE_PLAYERPROJECTILE_H
#define DPIBULLETHELLRE_PLAYERPROJECTILE_H

#include "../Projectile.h"

class PlayerProjectile : public Projectile {

    friend class Player;

public:
    PlayerProjectile(class Scene* scene, class Player* owner);
    void OnUpdate(float deltaTime) override;
    [[nodiscard]] bool IsOffScreen() const override;
    bool CheckCollision(Actor* other);


private:

    Player* mOwner;

    void ActivateProjectile();
    void DeactivateProjectile();

};


#endif //DPIBULLETHELLRE_PLAYERPROJECTILE_H
