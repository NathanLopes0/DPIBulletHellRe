//
// Created by gensh on 17/10/2025.
//

#pragma once

#include "../Motor/Math.h"
#include "PathAim.h"
#include "Amarra.h"
#include "Sprint.h"
#include "Ancora.h"
#include <vector>
#include <memory>
#include <string>
#include <utility>

class Projectile;

/// Define o contrato para qualquer "acessório" (behavior) que pode ser anexado a um Projétil.
class ProjectileBehavior {
public:
    virtual ~ProjectileBehavior() = default;

    virtual void update(class Projectile* projectile, float deltaTime) = 0;
    virtual bool isFinished() const { return false; }
};

/// Behavior que escreve a velocidade do projetil.
class ProjectileMotion : public ProjectileBehavior {
};

/// Behavior que modifica a velocidade ja existente, ou agenda quando ela comeca a valer. Varios
/// podem coexistir no mesmo projetil.
class ProjectileModifier : public ProjectileBehavior {
};

struct AccelerateBehavior : public ProjectileModifier {
    float accelerateDelay, accelerateSpeedPercent, elapsedTime;
    int accelerateSpeedValue;
    bool accelerated;

    /// Comportamento de acelerar o projétil. Se o segundo parametro for um int, aumentará
    /// exatamente aquela velocidade.
    explicit AccelerateBehavior(float delay = 0, float accelerateByXTimes = 0) :
    accelerateDelay(delay), accelerateSpeedPercent(accelerateByXTimes), elapsedTime(0.0f), accelerated(false),
    accelerateSpeedValue(-1) {};

    explicit AccelerateBehavior(float delay = 0, int accelerateByValue = 0) :
    accelerateDelay(delay), accelerateSpeedValue(accelerateByValue), elapsedTime(0.0f), accelerated(false),
    accelerateSpeedPercent(-1.0f) {};

    void update(Projectile* p, float deltaTime) override;
    bool isFinished() const override { return accelerated; }
};
struct SlowDownBehavior : public ProjectileModifier {
    float slowdownDelay, slowdownSpeedPercent, elapsedTime;
    int slowdownSpeedValue;
    bool slowedDown;

    explicit SlowDownBehavior(float delay = 0, float desaccelerateByXTimes = 0) :
            slowdownDelay(delay), slowdownSpeedPercent(desaccelerateByXTimes), elapsedTime(0.0f), slowdownSpeedValue(-1),
            slowedDown(false) {};
    explicit SlowDownBehavior(float delay = 0, int desaccelerateByValue = 0) :
            slowdownDelay(delay), slowdownSpeedPercent(-1.f), elapsedTime(0.0f), slowdownSpeedValue(desaccelerateByValue),
            slowedDown(false) {};

    void update(Projectile* p, float deltaTime) override;
    bool isFinished() const override { return slowedDown; }
};

struct ActivateBehavior : public ProjectileModifier {
    float activationDelay, elapsedTime;
    Vector2 activationVelocity;
    bool activated;

    explicit ActivateBehavior(const float delay = 0, const Vector2 activateVelocity = Vector2::Zero) :
    activationDelay(delay), elapsedTime(0.0f), activationVelocity(activateVelocity), activated(false) {};

    void update(Projectile* p, float deltaTime) override;
    bool isFinished() const override { return activated; }

};

/// Correcao de rota continua em direcao ao jogador, com forca que decai ate zero: o projetil
/// persegue e depois se compromete.
struct TrackingBehavior : public ProjectileMotion {

    explicit TrackingBehavior(float delay = 0.0f,
                                     float trackingStrength = 2.5f,
                                     float trackingTime = 2.0f)
        : startDelay(delay), trackingStrength(trackingStrength),
          trackingTime(trackingTime), elapsedTime(0.0f), finished(false) {}

    void update(Projectile* p, float deltaTime) override;
    bool isFinished() const override { return finished; }

    float startDelay;
    float trackingStrength;
    float trackingTime;
    float elapsedTime;
    bool finished;
};

/// Oscilacao lateral com amplitude decrescente em torno da trajetoria original: o projetil
/// serpenteia e vai se acalmando.
struct WobbleBehavior : public ProjectileMotion {

    explicit WobbleBehavior(float delay = 0.0f,
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

/// Conduz o projetil por uma lista de waypoints - "pathing" autoral. Como funciona.
struct PathBehavior : public ProjectileMotion {

    explicit PathBehavior(std::shared_ptr<const std::vector<Vector2>> waypoints,
                          float pathSpeed = 0.0f,
                          float delay = 0.0f,
                          Mira mira = Mira(),
                          int pararNoPonto = -1,
                          float pararPor = 0.0f)
        : waypoints(std::move(waypoints)), pathSpeed(pathSpeed), mira(mira),
          startDelay(delay), elapsedTime(0.0f), current(0),
          origin(Vector2::Zero), cosR(1.0f), sinR(0.0f),
          started(false), finished(false),
          pararNoPonto(pararNoPonto), pararPor(pararPor),
          paradaRestante(0.0f), paradaFeita(false) {}

    void update(Projectile* p, float deltaTime) override;
    bool isFinished() const override { return finished; }

    std::shared_ptr<const std::vector<Vector2>> waypoints;  // Flyweight: compartilhada
    float pathSpeed;
    Mira mira;   ///< regra que decide a orientacao do caminho
    float startDelay;
    float elapsedTime;
    size_t current;
    Vector2 origin;   // posicao capturada na ativacao
    float cosR, sinR; // rotacao capturada na ativacao
    bool started;
    bool finished;

    /// A parada num ponto do caminho.
    int   pararNoPonto;      ///< -1 = nunca para
    float pararPor;          ///< segundos
    float paradaRestante;
    bool  paradaFeita;       ///< So para uma vez, mesmo que o caminho volte ao ponto.
};

/// Re-aponta a velocidade para o jogador em instantes discretos.
struct MiraPeriodicaBehavior : public ProjectileMotion {

    explicit MiraPeriodicaBehavior(const RitmoDeCiclos& ritmo)
        : ritmo(ritmo), elapsedTime(0.0f), feitas(0) {}

    void update(Projectile* p, float deltaTime) override;
    bool isFinished() const override { return feitas >= ritmo.repeticoes; }

    RitmoDeCiclos ritmo;
    float elapsedTime;
    int feitas;
};

/// Alterna o modulo da velocidade entre uma investida rapida e uma pausa quase parada, repetidas
/// vezes.
struct PulsoDeVelocidadeBehavior : public ProjectileModifier {

    explicit PulsoDeVelocidadeBehavior(const RitmoDeCiclos& ritmo,
                                       float moduloInvestida = 800.0f,
                                       float moduloPausa = 15.0f)
        : ritmo(ritmo), moduloInvestida(moduloInvestida), moduloPausa(moduloPausa),
          elapsedTime(0.0f), terminou(false) {}

    void update(Projectile* p, float deltaTime) override;
    bool isFinished() const override { return terminou; }

    RitmoDeCiclos ritmo;
    float moduloInvestida, moduloPausa;
    float elapsedTime;
    bool terminou;
};

struct DeactivateBehavior : public ProjectileModifier {
    float elapsedTime, deactivationDelay;
    bool deactivated;

    explicit DeactivateBehavior(const float delay = 0) : elapsedTime(0.0f), deactivationDelay(delay), deactivated(false) {};

    void update(Projectile* p, float deltaTime) override;
    bool isFinished() const override { return deactivated; }
};

/// A Promise: o projetil espera pendente e, aos 'atraso' segundos, se resolve ou e rejeitado.
struct PromessaBehavior : public ProjectileModifier {

    PromessaBehavior(float atraso, float fracao, float velocidade, std::string escreve)
        : atraso(atraso), fracao(fracao), velocidade(velocidade), escreve(std::move(escreve)) {}

    void update(Projectile* p, float deltaTime) override;
    bool isFinished() const override { return assentou; }

    float atraso, fracao, velocidade;
    std::string escreve;
    float t = 0.0f;
    bool assentou = false;
};

/// A letra amarrada (ligare, "atar").
struct AmarrarBehavior : public ProjectileMotion {
    AmarrarBehavior(const Amarra::Corda& corda, std::string escreve)
        : corda(corda), escreve(std::move(escreve)) {}
    void update(Projectile* p, float deltaTime) override;
    bool isFinished() const override { return corda.cortada; }
    Amarra::Corda corda;
    std::string escreve;
};

/// Uma historia da sprint: vai na pilha que o cursor arrasta, se espalha para a vaga dela no quadro
/// e espera a revisao.
struct HistoriaDaSprintBehavior : public ProjectileMotion {
    HistoriaDaSprintBehavior(std::shared_ptr<const Sprint::Plano> plano, int indice, float queda)
        : plano(std::move(plano)), indice(indice), queda(queda) {}
    void update(Projectile* p, float deltaTime) override;
    bool isFinished() const override { return caiu; }
    std::shared_ptr<const Sprint::Plano> plano;
    int indice;
    float queda;
    float t = 0.0f;
    bool solta = false;
    bool noQuadro = false;
    bool caiu = false;
};

/// O cursor da sprint: a mao do professor.
struct CursorDaSprintBehavior : public ProjectileMotion {
    CursorDaSprintBehavior(std::shared_ptr<const Sprint::Plano> plano, std::string seta, std::string clique)
        : plano(std::move(plano)), seta(std::move(seta)), clique(std::move(clique)) {}
    void update(Projectile* p, float deltaTime) override;
    bool isFinished() const override { return acabou; }
    std::shared_ptr<const Sprint::Plano> plano;
    std::string seta, clique;
    float t = 0.0f;
    bool clicando = false;
    bool acabou = false;
};

/// Depois solta e cai reto, a 'queda' px/s - a janela que ninguem fechou.
struct AncorarBehavior : public ProjectileMotion {
    AncorarBehavior(Vector2 offset, float duracao, float queda)
        : offset(offset), duracao(duracao), queda(queda) {}
    void update(Projectile* p, float deltaTime) override;
    bool isFinished() const override { return soltou; }
    Vector2 offset;
    float duracao, queda;
    float t = 0.0f;
    bool soltou = false;
};
