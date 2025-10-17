//
// Created by nslop on 25/09/2024.
//

#pragma once

#include "../Actor.h"

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "../../Attacks/IAttackStrategy.h"

class Game;


class Boss : public Actor {
public:

    explicit Boss(Scene* scene);
    ~Boss() override = default;

    void AddAttackStrategy(std::unique_ptr<IAttackStrategy> strategy);
    void Start();

protected:

    void ExecuteAttack(size_t index);

    std::vector<std::unique_ptr<IAttackStrategy>> mAttackStrategies;

    virtual bool Movement0();
    virtual void Movement1();
    virtual void Movement2();
    virtual void Movement3();

    float mAtkTimer;
    std::map<std::string, float> mStateAtkTimers;
    void DecreaseAtkTimer(float deltaTime) { mAtkTimer -= deltaTime; }
    virtual void ResetAtkTimer() = 0;

};