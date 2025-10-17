//
// Created by nslop on 25/09/2024.
//

#pragma once

#include <string>
#include <map>
#include "../Actor.h"


class DrawAnimatedComponent;
class Game;

class Boss : public Actor {


public:

    Boss(Game* game);
    ~Boss() override = default;


    class BossState* GetCurrentState() { return mCurrentState; }

    //funções de conveniência
    [[nodiscard]] int GetWindowWidth() const;
    [[nodiscard]] int GetWindowHeight() const;
    [[nodiscard]] int GetSpriteHeight() const;
    [[nodiscard]] int GetSpriteWidth() const;

    virtual void Start();


protected:
    //variaveis para enviar powerUps na hora certa

    //componentes
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
    void DecreaseAtkTimer(const float deltaTime) { mAtkTimer -= deltaTime; }


};
