//
// Created by nslop on 14/12/2024.
//

#ifndef DPIBULLETHELLRE_BEHAVIORS_H
#define DPIBULLETHELLRE_BEHAVIORS_H

#include "../../Projectile.h"

class ProjectileBehavior {
public:
    virtual ~ProjectileBehavior() = default;
    virtual void update(class BossProjectile* projectile, float deltaTime) = 0;
};

struct HomingBehavior : public ProjectileBehavior {
    float homingDelay, homingSpeed, elapsedTime;
    bool homing;

    explicit HomingBehavior(float delay = 0, float speed = 0) :
    homingDelay(delay), homingSpeed(speed), elapsedTime(0.0f), homing(false) {};

    void update(class BossProjectile* p, float deltaTime) override;

};
struct AccelerateBehavior : public ProjectileBehavior {
    float accelerateDelay, accelerateSpeedPercent, elapsedTime;
    int accelerateSpeedValue;
    bool accelerated;

    explicit AccelerateBehavior(float delay = 0, float accelerateByXTimes = 0) :
    accelerateDelay(delay), accelerateSpeedPercent(accelerateByXTimes), elapsedTime(0.0f), accelerated(false),
    accelerateSpeedValue(-1) {};

    explicit AccelerateBehavior(float delay = 0, int accelerateByValue = 0) :
    accelerateDelay(delay), accelerateSpeedValue(accelerateByValue), elapsedTime(0.0f), accelerated(false),
    accelerateSpeedPercent(-1.0f) {};


    void update(BossProjectile* p, float deltaTime) override;
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

    void update(BossProjectile* p, float deltaTime) override;
};


#endif //DPIBULLETHELLRE_BEHAVIORS_H
