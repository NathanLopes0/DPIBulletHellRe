#pragma once

#include "../Components/AIComponents/FSMState.h"
#include <vector>
#include <memory>
#include <string>

// Forward-declarations
class IAttackStrategy;
class IMovementStrategy;
class Boss; // Precisamos saber que a classe "Boss" existe

/**
 * @class AttackState
 * @brief HERDA DE FSMState.
 * Controla os padrões de ataque e movimento de um Boss.
 */
class AttackState : public FSMState { // <-- HERDA DE FSMSTATE
public:
    /**
     * @param fsm O componente FSM "dono" (passado para o construtor base)
     * @param name O nome do estado (passado para o construtor base)
     * @param stateDuration O tempo total (em seg) que este estado ficará ativo (ex: 17.0f).
     * @param attackInterval O tempo (em seg) entre cada "pulso" de ataque (ex: 2.0f).
     * @param moveStrategy A estratégia de movimento que o Boss usará.
     */
    AttackState(FSMComponent* fsm, const std::string& name,
                float stateDuration, float attackInterval,
                std::unique_ptr<IMovementStrategy> moveStrategy);

    ~AttackState() override = default;

    // --- Montagem (Usado pela Factory) ---
    void AddAttackStrategy(std::unique_ptr<IAttackStrategy> strategy);

    // --- Ciclo de Vida (Overrides da base FSMState) ---
    void Start() override;
    void Update(float deltaTime) override;
    void Exit() override;

    /**
     * @brief A FSM vai chamar isso (baseado no seu .h)
     * Nós usaremos isso para checar o mStateDuration.
     */
    void HandleStateTransition(float stateTime) override;

private:
    // Ponteiro observador para o nosso Boss.
    // Preencher isso no Start() para evitar casts repetidos.
    Boss* mOwnerBoss;

    // --- Configuração (do Construtor) ---
    const float mStateDuration;
    const float mAttackInterval;
    std::unique_ptr<IMovementStrategy> mMovementStrategy;
    std::vector<std::unique_ptr<IAttackStrategy>> mAttackStrategies;

    // --- Timers Internos ---
    // 'stateTime' já é passado pela FSM
    float mAttackTimer;
};