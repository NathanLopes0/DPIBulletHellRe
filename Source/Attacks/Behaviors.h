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
            slowdownDelay(delay), slowdownSpeedPercent(desaccelerateByXTimes), elapsedTime(0.0f), slowdownSpeedValue(-1),
            slowedDown(false) {};
    explicit SlowDownBehavior(float delay = 0, int desaccelerateByValue = 0) :
            slowdownDelay(delay), slowdownSpeedPercent(-1.f), elapsedTime(0.0f), slowdownSpeedValue(desaccelerateByValue),
            slowedDown(false) {};

    void update(Projectile* p, float deltaTime) override; // Assinatura corrigida
    bool isFinished() const override { return slowedDown; }
};

struct ActivateBehavior : public ProjectileBehavior {
    float activationDelay, elapsedTime;
    Vector2 activationVelocity;
    bool activated;

    explicit ActivateBehavior(const float delay = 0, const Vector2 activateVelocity = Vector2::Zero) :
    activationDelay(delay), elapsedTime(0.0f), activationVelocity(activateVelocity), activated(false) {};

    void update(Projectile* p, float deltaTime) override;
    bool isFinished() const override { return activated; }

};

/**
 * @brief Correcao de rota CONTINUA em direcao ao jogador, com "taxa de
 * aprendizado" que decai ate zero. Tema: descida do gradiente.
 *
 * Diferente do HomingBehavior, que e um evento UNICO (aponta pro jogador uma
 * vez e se encerra), aqui a direcao e reajustada todo frame por uma fracao da
 * diferenca. O resultado visual e uma curva que fecha rapido no comeco e vai
 * "convergindo" ate travar - o projetil parece estar aprendendo a mirar.
 *
 * A taxa decai linearmente ao longo de convergenceTime (annealing). Quando
 * chega a zero, o behavior se encerra e para de custar CPU.
 *
 * O modulo da velocidade e SEMPRE preservado: so a direcao muda.
 */
struct GradientDescentBehavior : public ProjectileBehavior {

    /**
     * @param delay tempo ate comecar a corrigir a rota
     * @param learningRate quao forte e a correcao por segundo. Valores entre
     *        1.5 e 4.0 dao curvas legiveis; acima de ~6 o projetil praticamente
     *        gruda no jogador e o ataque fica injusto.
     * @param convergenceTime em quantos segundos a taxa decai a zero
     */
    explicit GradientDescentBehavior(float delay = 0.0f,
                                     float learningRate = 2.5f,
                                     float convergenceTime = 2.0f)
        : startDelay(delay), learningRate(learningRate),
          convergenceTime(convergenceTime), elapsedTime(0.0f), finished(false) {}

    void update(Projectile* p, float deltaTime) override;
    bool isFinished() const override { return finished; }

    float startDelay;
    float learningRate;
    float convergenceTime;
    float elapsedTime;
    bool finished;
};

/**
 * @brief Oscilacao lateral com amplitude decrescente em torno da trajetoria
 * original. Tema: overfitting (variancia alta que so estabiliza no fim).
 *
 * O projetil serpenteia em torno da direcao que tinha quando o behavior
 * ativou, com a amplitude caindo ate zero. Se dois projeteis vizinhos receberem
 * amplitudes de sinal oposto, eles se cruzam - visualmente muito bom para um
 * leque.
 *
 * A direcao base e capturada UMA vez na ativacao, e nao relida a cada frame.
 * Se fosse relida, a oscilacao realimentaria a si mesma e o projetil sairia
 * girando em espiral.
 */
struct OverfitBehavior : public ProjectileBehavior {

    /**
     * @param delay tempo ate comecar a oscilar
     * @param maxAngleDegrees amplitude maxima do desvio, em graus. Negativo
     *        inverte a fase (util para alternar projeteis vizinhos).
     * @param frequency oscilacoes completas por segundo
     * @param duration em quantos segundos a amplitude decai a zero
     */
    explicit OverfitBehavior(float delay = 0.0f,
                             float maxAngleDegrees = 45.0f,
                             float frequency = 1.5f,
                             float duration = 2.5f)
        : startDelay(delay), maxAngle(maxAngleDegrees), frequency(frequency),
          duration(duration), elapsedTime(0.0f), baseDirection(Vector2::Zero),
          baseSpeed(0.0f), started(false), finished(false) {}

    void update(Projectile* p, float deltaTime) override;
    bool isFinished() const override { return finished; }

    float startDelay;
    float maxAngle;
    float frequency;
    float duration;
    float elapsedTime;
    Vector2 baseDirection;
    float baseSpeed;
    bool started;
    bool finished;
};

struct DeactivateBehavior : public ProjectileBehavior {
    float elapsedTime, deactivationDelay;
    bool deactivated;

    explicit DeactivateBehavior(const float delay = 0) : elapsedTime(0.0f), deactivationDelay(delay), deactivated(false) {};

    void update(Projectile* p, float deltaTime) override;
    bool isFinished() const override { return deactivated; }
};