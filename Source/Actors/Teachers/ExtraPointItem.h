//
// Created by gensh on 24/11/2025.
//

#pragma once
#include "../Actor.h"

class ExtraPointItem : public Actor {
public:
    explicit ExtraPointItem(Scene* scene);

    void OnUpdate(float deltaTime) override;

private:
    const float FALL_SPEED = 100.0f; // Velocidade de queda
};
