//
// Created by gensh on 19/10/2025.
//

#pragma once

class Boss;

class IMovementStrategy {
public:
    virtual ~IMovementStrategy() = default;

    virtual void InitializeBoss(Boss* boss) {};

    virtual void Update(Boss* boss, float deltaTime) = 0;

};