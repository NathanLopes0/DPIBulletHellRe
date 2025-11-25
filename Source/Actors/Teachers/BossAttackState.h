//
// Created by gensh on 01/11/2025.
//

#ifndef DPIBULLETHELLRE_BOSSATTACKSTATE_H
#define DPIBULLETHELLRE_BOSSATTACKSTATE_H

#include "../../Components/AIComponents/FSMState.h"
#include <string>

/**
 * @class BossAttackState
 * @brief Um "estado" fino que só gerencia transições.
 *
 * Esta classe é um "Vigia". Seu único trabalho é
 * contar o tempo e checar condições para a transição.
 */
class BossAttackState : public FSMState {

public:
    BossAttackState(FSMComponent* fsm,
                    const std::string& name,
                    float duration,
                    const std::string& nextStateName);

    ~BossAttackState() override = default;

    /** @brief Sobrescreve GetDuration para retornar a duração deste estado. */
    [[nodiscard]] float GetDuration() const override { return mDuration; }

    void HandleStateTransition(float stateTime) override;

private:
    float mDuration;
    std::string mNextStateName;

};


#endif //DPIBULLETHELLRE_BOSSATTACKSTATE_H