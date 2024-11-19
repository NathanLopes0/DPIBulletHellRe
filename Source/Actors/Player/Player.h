//
// Created by nslop on 03/10/2024.
//

#ifndef DPIBULLETHELLRE_PLAYER_H
#define DPIBULLETHELLRE_PLAYER_H

#include "../Actor.h"

class Player : public Actor {

public:
    Player(class Scene* scene);

    //Process Input -> Update, sobrescritas de Actor para Player
    void OnProcessInput(const Uint8* keyState) override;
    void OnUpdate(float deltaTime) override;



private:
    class RigidBodyComponent* mRigidBodyComponent;
    class DrawAnimatedComponent* mDrawComponent;
    float playerSpeed;
    bool mMoving;                //variavel para ver se o jogador está se movendo. Usada para mudar a animação de acordo.


    //OnProcessInput Sub-functions
    void MoveInput(const Uint8 *state);

    void ShootInput(const Uint8 *state);

    void SpecialInput(const Uint8 *state);

    void HandleAnimation();
};


#endif //DPIBULLETHELLRE_PLAYER_H
