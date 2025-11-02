//
// Created by gensh on 17/10/2025.
//

#pragma once

#include "../Math.h"

class Projectile;

/**
 * @interface ProjectileBehavior
 * @brief Define o contrato para qualquer "acessório" (behavior)
 * que pode ser anexado a um Projétil.
 */
class ProjectileBehavior {
public:
    virtual ~ProjectileBehavior() = default;

    virtual void update(class Projectile* projectile, float deltaTime) = 0;
    virtual bool isFinished() const { return false; }
};

struct HomingBehavior : public ProjectileBehavior {
    float homingDelay, homingSpeed, elapsedTime;
    bool homing;

    /**
     * @brief comportamento de seguir o jogador (instantaneamente ou depois de um tempo)
     * @param delay quanto tempo até o projétil começar a seguir o Player (se ignorado, começará seguindo o jogador)
     * @param speed velocidade que o projétil seguirá o jogador (se ignorado, continuará com a mesma velocidade)
     */
    explicit HomingBehavior(float delay = 0, float speed = 0) :
    homingDelay(delay), homingSpeed(speed), elapsedTime(0.0f), homing(false) {};

    void update(Projectile* p, float deltaTime) override;
    bool isFinished() const override { return homing; }

};

struct AccelerateBehavior : public ProjectileBehavior {
    float accelerateDelay, accelerateSpeedPercent, elapsedTime;
    int accelerateSpeedValue;
    bool accelerated;

    /**
     * @brief comportamento de acelerar o projétil. Se o segundo parametro for um INT, aumentará exatamente aquela velocidade.
     * Se for um FLOAT, multiplicará a velocidade do projétil pelo valor passado (1.5f aumentará a velocidade em 50%, por exemplo)
     * @param delay quanto tempo até o projétil acelerar
     * @param accelerateByXTimes valor que será multiplicado pela velocidade
     */
    explicit AccelerateBehavior(float delay = 0, float accelerateByXTimes = 0) :
    accelerateDelay(delay), accelerateSpeedPercent(accelerateByXTimes), elapsedTime(0.0f), accelerated(false),
    accelerateSpeedValue(-1) {};

    explicit AccelerateBehavior(float delay = 0, int accelerateByValue = 0) :
    accelerateDelay(delay), accelerateSpeedValue(accelerateByValue), elapsedTime(0.0f), accelerated(false),
    accelerateSpeedPercent(-1.0f) {};


    void update(Projectile* p, float deltaTime) override; // Assinatura corrigida
    bool isFinished() const override { return accelerated; }
};
struct SlowDownBehavior : public ProjectileBehavior {
    float slowdownDelay, slowdownSpeedPercent, elapsedTime;
    int slowdownSpeedValue;
    bool slowedDown;


    explicit SlowDownBehavior(float delay = 0, float desaccelerateByXTimes = 0) :
            slowdownDelay(delay), slowdownSpeedPercent(desaccelerateByXTimes), elapsedTime(0.0f), slowedDown(false),
            slowdownSpeedValue(-1) {};
    explicit SlowDownBehavior(float delay = 0, int desaccelerateByValue = 0) :
            slowdownDelay(delay), slowdownSpeedValue(desaccelerateByValue), elapsedTime(0.0f), slowedDown(false),
            slowdownSpeedPercent(-1.f) {};

    void update(Projectile* p, float deltaTime) override; // Assinatura corrigida
    bool isFinished() const override { return slowedDown; }
};

struct ActivateBehavior : public ProjectileBehavior {
    float activationDelay, elapsedTime;
    Vector2 activationVelocity;
    bool activated;

    explicit ActivateBehavior(float delay = 0, Vector2 activateVelocity = Vector2::Zero) :
    activationDelay(delay), elapsedTime(0.0f), activated(false), activationVelocity(activateVelocity) {};

    void update(Projectile* p, float deltaTime) override; // Assinatura corrigida
    bool isFinished() const override { return activated; }

};