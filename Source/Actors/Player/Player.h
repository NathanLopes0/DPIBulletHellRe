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
    class CircleColliderComponent* mColliderComponent;

    std::vector<class PlayerProjectile*> mProjectiles;      //vetor de projéteis que será alocado no construtor,
                                                            // para não fazer alocação no meio do jogo.

    //variaveis de jogador
    float playerSpeed;
    float atkTimer;
    bool mMoving;                       //Ve se e jogadore está se movendo. Usada para mudar a animação.
    bool mIsBlinkingInvincible;
    float mIsInvincibleSeconds;         //Ativa quando elu colide (ou, no futuro, powerup quem sabe)



    //OnProcessInput Sub-functions, and its sub-functions
    void MoveInput(const Uint8 *state);
    void ShootInput(const Uint8 *state);
        void Shoot();
    void SpecialInput(const Uint8 *state);


    //OnUpdate Sub-functions ----------------- //
    void HandleAnimation();
    void DecreaseAtkTimer(float deltaTime);
    int CheckCollision();
    void OnCollision(int collidedWith, float deltaTime);
    // --------------------------------------- //

    void BorderLimitCheck();

    void Blink(float deltaTime);

    void DeduceGrade(float d, float d1);

    void SetInvincibleSeconds(float d);
};


#endif //DPIBULLETHELLRE_PLAYER_H
