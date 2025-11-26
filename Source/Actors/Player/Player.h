//
// Created by nslop on 03/10/2024.
//

#ifndef DPIBULLETHELLRE_PLAYER_H
#define DPIBULLETHELLRE_PLAYER_H

#include "../Actor.h"

class PlayerProjectile;
class PlayerOrb;

class Player : public Actor {

public:
    explicit Player(Scene* scene);

    //Process Input -> Update, sobrescritas de Actor para Player
    void OnProcessInput(const Uint8* keyState) override;

    void UpdateOverload(float deltaTime);

    void OnUpdate(float deltaTime) override;

    void SetIsInvincible(const bool invincible) { isInvincible = invincible; }
    [[nodiscard]] bool GetIsInvincible() const { return isInvincible; }

    [[nodiscard]] int GetExtraPoints() const { return mExtraPoints;}
    void AddExtraPoint();


private:

    //variaveis de jogador
    float playerSpeed;
    float atkTimer;
    float invincibleTimer;
    bool isInvincible;
    bool mMoving;                       //ve se o jogador está se movendo. Usada para mudar a animação
    int mExtraPoints = 1;
    float mOverloadTimer = 0.0f;
    const float OVERLOAD_DURATION = 3.f;
    float mPulseTimer = 0.0f;



    //OnProcessInput Sub-functions, and its sub-functions
    void MoveInput(const Uint8 *state);
    void ShootInput(const Uint8 *state);
        void Shoot();
    void SpecialInput(const Uint8 *state);


    //OnUpdate Sub-functions
    void InvincibleUpdate(float deltaTime);
    void HandleAnimation();
    void DecreaseAtkTimer(float deltaTime);

    void BorderLimitCheck();

};


#endif //DPIBULLETHELLRE_PLAYER_H
