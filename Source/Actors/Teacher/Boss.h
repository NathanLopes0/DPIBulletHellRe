//
// Created by nslop on 25/09/2024.
//

#ifndef DPIBULLETHELLRE_BOSS_H
#define DPIBULLETHELLRE_BOSS_H

#include "../Actor.h"
#include "../../Game.h"
#include "../../Scenes/Scene.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Components/AIComponents/FSMComponent.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Components/ColliderComponents/CircleColliderComponent.h"

class Boss : public Actor {


public:

    Boss(class Scene* scene, const std::string& spritePath, const std::string& dataPath);
    ~Boss();

    class BossState* GetCurrentState() { return mCurrentState; }

    //funções de pegar Largura e Altura da sprite e do jogo
    int GetWindowsWidth() { return mScene->GetGame()->GetWindowWidth(); }
    int GetWindowsHeight() { return mScene->GetGame()->GetWindowHeight(); }
    int GetSpriteHeight() { return GetComponent<DrawAnimatedComponent>()->GetSpriteHeight(); }
    int GetSpriteWidth() { return GetComponent<DrawAnimatedComponent>()->GetSpriteWidth(); }

    virtual void Start();


protected:
    //variaveis para enviar powerUps na hora certa

    //componentes
    RigidBodyComponent* mRigidBodyComponent;
    DrawAnimatedComponent* mDrawComponent;
    class CircleColliderComponent* mColliderComponent;
    class FSMComponent* mFSMComponent;
    class BossState* mCurrentState;
    float mAtkTimer;


    //Como Fazer a lógica dos intervalos de ataque? dentro das funções de Attack ou alguma lógica externa que CHAMA a função
    //de ataque em um intervalo que depende do boss? Acho que dentro da lógica de ataque é melhor.

    //4 movimentos e 3 ataques, para cada Estado.
    virtual bool Movement0() = 0;
    virtual void Movement1() = 0;
    virtual void Movement2() = 0;
    virtual void Movement3() = 0;
    virtual void Attack1() = 0;
    virtual void Attack2() = 0;
    virtual void Attack3() = 0;

    void StateActions();

    //funções e estruturas para manipular os timers de ataque (velocidade de ataque) de cada estado dos bosses.
    virtual void ResetAtkTimer() = 0;
    std::map<std::string, float> mStateAtkTimers;
    void DecreaseAtkTimer(float deltaTime) { mAtkTimer -= deltaTime; }


};


#endif //DPIBULLETHELLRE_BOSS_H
