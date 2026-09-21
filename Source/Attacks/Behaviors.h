//
// Created by gensh on 17/10/2025.
//

#pragma once

#include "../Math.h"
#include "PathAim.h"
#include <vector>
#include <memory>
#include <utility>

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

/**
 * @brief Behavior que ESCREVE a velocidade do projetil.
 *
 * No maximo UM por projetil: dois escritores brigariam frame a frame e o
 * ultimo a rodar venceria, o que produz movimento aleatorio e dificil de
 * depurar. Projectile::insertMotion garante a exclusividade.
 *
 * Sao Motion: Homing, Tracking, Wobble e Path.
 */
class ProjectileMotion : public ProjectileBehavior {
};

/**
 * @brief Behavior que MODIFICA a velocidade ja existente, ou agenda quando ela
 * comeca a valer. Varios podem coexistir no mesmo projetil.
 *
 * Sao Modifier: Accelerate, SlowDown, Activate e Deactivate.
 *
 * Esta separacao existe porque a regra "Wobble e Path nao compoem com Tracking,
 * Accelerate e SlowDown" era so um comentario de cabecalho: quem combinasse
 * errado compilava e descobria na tela. Agora nao compila.
 */
class ProjectileModifier : public ProjectileBehavior {
};

/**
 * SUBSTITUIDO na fase 2, a ser migrado na fase 3.
 *
 * Desde que PathBehavior aceita Mira, um homing e exprimivel como caminho:
 *     insertMotion<PathBehavior>(PathShapes::Reta(800.f), velocidade, atraso,
 *                                Mira(Mira::MirarNoJogador));
 * Mantido por enquanto porque Ricardo e Salles dependem dele e a migracao
 * merece teste comparando as duas trajetorias antes de apagar esta classe.
 * NAO use em codigo novo.
 */
struct HomingBehavior : public ProjectileMotion {
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

struct AccelerateBehavior : public ProjectileModifier {
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

    void update(Projectile* p, float deltaTime) override; // Assinatura corrigida
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

/**
 * @brief Correcao de rota CONTINUA em direcao ao jogador, com forca que decai
 * ate zero: o projetil persegue e depois se compromete.
 *
 * Diferente do HomingBehavior, que e um evento UNICO (aponta pro jogador uma
 * vez e se encerra), aqui a direcao e reajustada todo frame por uma fracao da
 * diferenca. O resultado visual e uma curva que fecha rapido no comeco e vai
 * afrouxando ate travar.
 *
 * A forca decai linearmente ao longo de trackingTime, o que da ao jogador uma
 * janela previsivel para desviar: fugir cedo nao resolve, fugir tarde sim.
 *
 * O modulo da velocidade e SEMPRE preservado: so a direcao muda.
 */
struct TrackingBehavior : public ProjectileMotion {

    /**
     * @param delay tempo ate comecar a corrigir a rota
     * @param trackingStrength quao forte e a correcao por segundo. Valores entre
     *        1.5 e 4.0 dao curvas legiveis; acima de ~6 o projetil praticamente
     *        gruda no jogador e o ataque fica injusto.
     * @param trackingTime em quantos segundos a forca decai a zero
     */
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

/**
 * @brief Oscilacao lateral com amplitude decrescente em torno da trajetoria
 * original: o projetil serpenteia e vai se acalmando.
 *
 * Se dois projeteis vizinhos receberem amplitudes de sinal oposto, eles se
 * cruzam - visualmente muito bom para um leque.
 *
 * A direcao base e capturada UMA vez na ativacao, e nao relida a cada frame.
 * Se fosse relida, a oscilacao realimentaria a si mesma e o projetil sairia
 * girando em espiral.
 *
 * ATENCAO - NAO COMBINE com TrackingBehavior, AccelerateBehavior nem
 * SlowDownBehavior no mesmo projetil. Este behavior REESCREVE a velocidade
 * todo frame a partir da direcao e do modulo capturados na ativacao, entao
 * qualquer alteracao feita por outro behavior e desfeita no frame seguinte.
 * Use-o sozinho.
 */
struct WobbleBehavior : public ProjectileMotion {

    /**
     * @param delay tempo ate comecar a oscilar
     * @param maxAngleDegrees amplitude maxima do desvio, em graus. Negativo
     *        inverte a fase (util para alternar projeteis vizinhos).
     * @param frequency oscilacoes completas por segundo
     * @param duration em quantos segundos a amplitude decai a zero
     */
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

/**
 * @brief Conduz o projetil por uma lista de waypoints - "pathing" autoral.
 *
 * COMO FUNCIONA
 * Os waypoints sao OFFSETS em "espaco de caminho", nao coordenadas de tela:
 *   +X = a direcao em que o projetil ja estava viajando quando o behavior
 *        ativou;
 *   +Y = a perpendicular, a direita desse movimento.
 * Na ativacao, o behavior captura a posicao e a direcao atuais do projetil e
 * gira o caminho inteiro para alinhar com elas. A consequencia util: o MESMO
 * caminho disparado por um CircleSpreadAttack vira um caminho diferente para
 * cada projetil do anel, ja que cada um tem sua propria direcao inicial.
 *
 * POR QUE ESCREVE VELOCIDADE, E NAO POSICAO
 * Seria mais simples chamar SetPosition a cada frame, mas o RigidBodyComponent
 * roda ANTES dos behaviors (Actor::Update atualiza componentes e so depois
 * chama OnUpdate) e faz position += velocity * dt. Escrever posicao direto
 * colocaria os dois para brigar todo frame. Definindo a velocidade que aponta
 * ao proximo waypoint, o RigidBody continua sendo o unico que escreve posicao,
 * e colisao, IsOffScreen e todo o resto seguem funcionando sem saber que existe
 * um caminho.
 *
 * AO TERMINAR O CAMINHO o projetil segue RETO na ultima direcao. Ele nunca
 * para: um projetil parado nunca satisfaz IsOffScreen(), nunca e devolvido ao
 * pool e fica ocupando a tela para sempre.
 *
 * ATENCAO - EXCLUSIVO. Vale aqui a mesma regra do WobbleBehavior: behaviors que
 * ESCREVEM velocidade nao compoem com os que a modificam. Nao combine com
 * TrackingBehavior, AccelerateBehavior, SlowDownBehavior nem WobbleBehavior.
 * Activate/DeactivateBehavior continuam seguros, desde que o Activate dispare
 * ANTES deste (senao a direcao capturada seria zero).
 */
struct PathBehavior : public ProjectileMotion {

    /**
     * @param waypoints Offsets em espaco de caminho, na ordem de percurso.
     *        Use as funcoes de PathShapes.h em vez de digitar na mao.
     * @param pathSpeed Velocidade ao longo do caminho. 0 = mantem o modulo da
     *        velocidade que o projetil ja tinha.
     * @param delay Segundos ate comecar a seguir o caminho.
     * @param mira Regra de orientacao. O padrao alinha o caminho com a direcao
     *        de voo, que e o comportamento historico - por isso todas as
     *        chamadas existentes continuam validas sem alteracao.
     */
    explicit PathBehavior(std::shared_ptr<const std::vector<Vector2>> waypoints,
                          float pathSpeed = 0.0f,
                          float delay = 0.0f,
                          Mira mira = Mira())
        : waypoints(std::move(waypoints)), pathSpeed(pathSpeed), mira(mira),
          startDelay(delay), elapsedTime(0.0f), current(0),
          origin(Vector2::Zero), cosR(1.0f), sinR(0.0f),
          started(false), finished(false) {}

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
};

struct DeactivateBehavior : public ProjectileModifier {
    float elapsedTime, deactivationDelay;
    bool deactivated;

    explicit DeactivateBehavior(const float delay = 0) : elapsedTime(0.0f), deactivationDelay(delay), deactivated(false) {};

    void update(Projectile* p, float deltaTime) override;
    bool isFinished() const override { return deactivated; }
};