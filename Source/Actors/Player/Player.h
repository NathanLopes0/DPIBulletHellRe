//
// Created by nslop on 03/10/2024.
//

#ifndef DPIBULLETHELLRE_PLAYER_H
#define DPIBULLETHELLRE_PLAYER_H

#include "../Actor.h"

class PlayerProjectile;

class Player : public Actor {

public:
    explicit Player(Scene* scene);

    //Process Input -> Update, sobrescritas de Actor para Player
    void OnProcessInput(const Uint8* keyState) override;
    void OnUpdate(float deltaTime) override;


private:

    //variaveis de jogador
    float playerSpeed;
    float atkTimer;
    bool mMoving;                       //ve se o jogador está se movendo. Usada para mudar a animação.


    //OnProcessInput Sub-functions, and its sub-functions
    void MoveInput(const Uint8 *state);
    void ShootInput(const Uint8 *state);
        void Shoot();
    void SpecialInput(const Uint8 *state);


    //OnUpdate Sub-functions
    void HandleAnimation();
    void DecreaseAtkTimer(float deltaTime);

    void BorderLimitCheck();
};


#endif //DPIBULLETHELLRE_PLAYER_H
