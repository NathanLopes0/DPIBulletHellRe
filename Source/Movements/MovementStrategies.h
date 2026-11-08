#pragma once

#include "../Motor/Math.h"
#include "../Components/RigidBodyComponent.h"
#include "../Actors/Teachers/Boss.h"
#include "../Motor/Random.h"

// ---- Interface ----
class IMovementStrategy {
public:
    virtual ~IMovementStrategy() = default;
    virtual void Initialize(Boss* boss) {}
    virtual void Update(Boss* boss, float deltaTime) = 0;
};

// ---- Estratégia 1: random wander ----
class RandomWanderStrategy : public IMovementStrategy {
public:
    RandomWanderStrategy(float interval, float speed, float margin = 100.0f);

    void Initialize(Boss* boss) override;
    void Update(Boss* boss, float deltaTime) override;

private:
    void PickNewTarget(Boss* boss);

    float mHoverTimer = 0.0f;

    // Constantes de Flutuação.
    const float HOVER_FREQ = 2.0f;   // Rapidez da flutuação
    const float HOVER_AMP = 25.0f;   // Força da flutuação

    enum State { Moving, Idle };
    State mState;
    Vector2 mTargetPos;
    float mChangeInterval;
    float mMoveSpeed;
    float mMargin;
    float mWaitTimer;
    bool mIsMoving;
};

// ---- Estratégia 2: hover above player ----
class HoverAbovePlayerStrategy : public IMovementStrategy {
public:
    HoverAbovePlayerStrategy(float speed = 150.0f, float yLevel = 150.0f);

    void Update(Boss* boss, float deltaTime) override;

private:
    float mSpeed;
    float mPreferredY;

    float mHoverTimer = 0.0f;
};

// ---- Estratégia 3: static center ----
class GoToCenterStrategy : public IMovementStrategy {
public:
    void Initialize(Boss* boss) override;
    void Update(Boss* boss, float deltaTime) override;
private:
    bool mReached = false;
};

/// As tres de baixo existem para o movimento ser o ataque.

/// Gira o chefe num circulo, sem parar. O gesto da estrela de kleene: (ab)* e um laco que repete, e
/// a orbita E o laco.
class OrbitarStrategy : public IMovementStrategy {
public:
    explicit OrbitarStrategy(float raio = 220.0f, float voltasPorSegundo = 0.18f);

    void Initialize(Boss* boss) override;
    void Update(Boss* boss, float deltaTime) override;

private:
    Vector2 mCentro = Vector2::Zero;
    float mRaio;
    float mVoltasPorSegundo;
    float mAngulo = 0.0f;
};

/// Vai e volta na horizontal, de margem a margem. O gesto de a^n b^n e da fita de Turing.
class VarreduraStrategy : public IMovementStrategy {
public:
    explicit VarreduraStrategy(float velocidade = 260.0f, float margem = 140.0f);

    void Initialize(Boss* boss) override;
    void Update(Boss* boss, float deltaTime) override;

private:
    float mVelocidade;
    float mMargem;
    float mDirecao = 1.0f;
    float mAltura = 0.0f;
};

/// Desce e sobe, repetindo. O gesto da pilha: empilhar e descer, desempilhar e subir, e a
/// profundidade do chefe e a altura da pilha.
class MergulhoStrategy : public IMovementStrategy {
public:
    explicit MergulhoStrategy(float velocidade = 200.0f, float profundidade = 300.0f);

    void Initialize(Boss* boss) override;
    void Update(Boss* boss, float deltaTime) override;

private:
    float mVelocidade;
    float mProfundidade;
    float mTopo = 0.0f;
    float mDirecao = 1.0f;

    /// A deriva lateral: devagar perto do mergulho, para o gesto continuar
    /// sendo vertical e ainda assim a coluna andar pelo campo.
    float mDeriva = 1.0f;
    static constexpr float kVelocidadeDaDeriva = 70.0f;
    static constexpr float kMargemDaDeriva = 170.0f;
};

/// Desce a escada do modelo cascata: um degrau a cada 'segundosPorDegrau', da esquerda para a
/// direita, e nao sobe mais. O gesto da cascata.
class EscadaStrategy : public IMovementStrategy {
public:
    EscadaStrategy(float segundosPorDegrau = 3.4f, float degraus = 5.0f);
    void Initialize(Boss* boss) override;
    void Update(Boss* boss, float deltaTime) override;
private:
    float mSegundosPorDegrau;
    int mDegraus;
    float mTempo = 0.0f;
    /// A velocidade com que ele vai de um degrau ao seguinte: depressa, para o
    /// degrau ser um degrau e nao uma rampa.
    static constexpr float kVelocidadeEntreDegraus = 420.0f;
};

/// Anda de coluna em coluna, uma a cada 'segundosPorPasso', ate a ponta e de volta - um bit num
/// registrador de deslocamento. O gesto do deslocador.
class DeslocamentoStrategy : public IMovementStrategy {
public:
    DeslocamentoStrategy(float segundosPorPasso = 0.6f, float colunas = 16.0f);
    void Initialize(Boss* boss) override;
    void Update(Boss* boss, float deltaTime) override;
private:
    float mSegundosPorPasso;
    int mColunas;
    int mInicio = 0;
    float mAltura = 0.0f;
    float mTempo = 0.0f;
    /// Quantas colunas de cada borda ele nao pisa: a ponta da palavra e o
    /// canto da tela.
    static constexpr int kColunasDaBorda = 2;
    /// Depressa, para o passo ser um passo e nao um deslizar.
    static constexpr float kVelocidadeDoPasso = 500.0f;
};

/// Anda na onda quadrada do clock: no nivel alto em cima, no baixo embaixo, saltando nas bordas - e
/// de lado a lado, devagar. O gesto do clock.
class OndaQuadradaStrategy : public IMovementStrategy {
public:
    explicit OndaQuadradaStrategy(float amplitude = 60.0f, float velocidade = 50.0f);
    void Initialize(Boss* boss) override;
    void Update(Boss* boss, float deltaTime) override;
private:
    float mAmplitude;
    float mVelocidade;
    float mDirecao = 1.0f;
    float mAltura = 0.0f;
    static constexpr float kMargem = 180.0f;
    /// Quao depressa ele vai de um nivel ao outro: o salto da borda.
    static constexpr float kRapidezDaBorda = 30.0f;
};

/// Vai aonde o proprio professor diz (Boss::AlvoDoMovimento), e para la.
class ConduzidoStrategy : public IMovementStrategy {
public:
    explicit ConduzidoStrategy(float velocidade = 900.0f);
    void Update(Boss* boss, float deltaTime) override;
private:
    float mVelocidade;
};
