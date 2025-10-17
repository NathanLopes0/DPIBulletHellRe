//
// Created by nslop on 18/11/2024.
//

#ifndef DPIBULLETHELLRE_PROJECTILE_H
#define DPIBULLETHELLRE_PROJECTILE_H

#include "Actor.h"


class Projectile : public Actor {

public:
    explicit Projectile(Scene* scene, Actor* owner);
    ~Projectile() override = default;

    // Ciclo de Vida
    void OnUpdate(float deltaTime) override;
    void OnCollision(Actor* other) override;


    void SetForwardSpeed(float newSpeed) { mForwardSpeed = newSpeed; }
    [[nodiscard]] float GetForwardSpeed() const { return mForwardSpeed; }

    [[nodiscard]] Actor* GetOwner() const { return mOwner; }


protected:

    Actor* mOwner; // Ponteiro observador pra quem criou esse projétil
    float mForwardSpeed;


    //Sub-funções de OnUpdate
    [[nodiscard]] virtual bool IsOffScreen() const = 0;

};


#endif //DPIBULLETHELLRE_PROJECTILE_H
