#pragma once

#include "../Math.h"
#include "../Components/RigidBodyComponent.h"
#include "../Actors/Teachers/Boss.h"
#include "../Random.h"

// --- INTERFACE ---
class IMovementStrategy {
public:
    virtual ~IMovementStrategy() = default;
    virtual void Initialize(Boss* boss) {}
    virtual void Update(Boss* boss, float deltaTime) = 0;
};

// --- ESTRATÉGIA 1: RANDOM WANDER ---
class RandomWanderStrategy : public IMovementStrategy {
public:
    RandomWanderStrategy(float interval, float speed, float margin = 100.0f);

    void Initialize(Boss* boss) override;
    void Update(Boss* boss, float deltaTime) override;

private:
    void PickNewTarget(Boss* boss);

    float mHoverTimer = 0.0f;

    // Constantes de Flutuação
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

// --- ESTRATÉGIA 2: HOVER ABOVE PLAYER ---
class HoverAbovePlayerStrategy : public IMovementStrategy {
public:
    HoverAbovePlayerStrategy(float speed = 150.0f, float yLevel = 150.0f);

    void Update(Boss* boss, float deltaTime) override;

private:
    float mSpeed;
    float mPreferredY;

    float mHoverTimer = 0.0f;
};

// --- ESTRATÉGIA 3: STATIC CENTER ---
class GoToCenterStrategy : public IMovementStrategy {
public:
    void Initialize(Boss* boss) override;
    void Update(Boss* boss, float deltaTime) override;
private:
    bool mReached = false;
};

/**
 * AS TRES DE BAIXO EXISTEM PARA O MOVIMENTO SER O ATAQUE.
 *
 * As tres primeiras estrategias tratam o chefe como um atirador que por acaso
 * anda: ele vagueia, persegue ou fica parado, e o padrao de tiro e outra coisa.
 * Para o Vladimir isso nao servia - com ele parado no centro, cada rajada caia
 * na mesma coluna e a tela inteira ficava vazia dos lados.
 *
 * O CHEFE E UMA CANETA. Uma palavra nao se escreve empilhando letras num ponto:
 * se escreve com a caneta ANDANDO, e as letras ficam deitadas ao longo do
 * caminho. Cada uma destas tres e o gesto de escrita de uma fase - o laco, o ir
 * e voltar, o descer e subir - e e por isso que elas levam o nome do gesto e
 * nao do formato.
 */

/**
 * @brief Gira o chefe num circulo, sem parar.
 *
 * O GESTO DA ESTRELA DE KLEENE: (ab)* e um laco que repete, e a orbita E o
 * laco. Disparando enquanto gira, os simbolos ficam distribuidos em torno do
 * anel inteiro em vez de numa coluna, e a repeticao da volta e visivel como
 * repeticao.
 *
 * @param raio             em pixels
 * @param voltasPorSegundo quantas voltas completas por segundo
 */
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

/**
 * @brief Vai e volta na horizontal, de margem a margem.
 *
 * O GESTO DE a^n b^n E DA FITA DE TURING. Na fase 2 o chefe escreve os "a"
 * indo e os "b" voltando: a mesma distancia nos dois sentidos, e o MOVIMENTO e
 * a contagem. Na fase final e a cabeca de leitura varrendo a fita.
 *
 * @param velocidade em px/s
 * @param margem     quanto parar antes da borda, em pixels
 */
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

/**
 * @brief Desce e sobe, repetindo.
 *
 * O GESTO DA PILHA: empilhar e descer, desempilhar e subir, e a PROFUNDIDADE
 * do chefe e a altura da pilha. Descendo enquanto dispara, ele deixa os
 * simbolos em alturas diferentes - e a pilha fica desenhada no campo.
 *
 * @param velocidade   em px/s
 * @param profundidade quanto descer a partir da altura em que comecou
 */
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