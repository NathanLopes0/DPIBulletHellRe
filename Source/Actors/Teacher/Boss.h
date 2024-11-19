//
// Created by nslop on 25/09/2024.
//

#ifndef DPIBULLETHELLRE_BOSS_H
#define DPIBULLETHELLRE_BOSS_H

#include "../../Game.h"
#include "../../Scenes/Scene.h"
#include "../Actor.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Components/AIComponents/FSMComponent.h"

class Boss : public Actor {

public:
    Boss(class Scene* scene);
    ~Boss();

    //4 movimentos e 3 ataques, para cada Estado.
    virtual bool Movement0() = 0;
    virtual void Movement1() = 0;
    virtual void Movement2() = 0;
    virtual void Movement3() = 0;
    virtual void Attack1() = 0;
    virtual void Attack2() = 0;
    virtual void Attack3() = 0;

    class BossState* GetCurrentState() { return mCurrentState; }

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
    class FSMComponent* mFSMComponent;
    class BossState* mCurrentState;


};


#endif //DPIBULLETHELLRE_BOSS_H
