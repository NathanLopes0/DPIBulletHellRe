//
// Created by nslop on 09/10/2024.
//

#ifndef DPIBULLETHELLRE_BOSSSTATE_H
#define DPIBULLETHELLRE_BOSSSTATE_H

#include "../../../Components/AIComponents/FSMState.h"

class BossState : public FSMState {

public:
    BossState(FSMComponent* stateMachine, const std::string& name);
    void Update(float deltaTime) override;

    float GetMaxStateTime() const { return mMaxStateTime; }
    void SetState(FSMState *pState);
    void SetMaxStateTime(float newMaxStateTime) { mMaxStateTime = newMaxStateTime; }
    float GetStateTime() const;

protected:

    float mMaxStateTime;
    class Boss* mBoss;
};


#endif //DPIBULLETHELLRE_BOSSSTATE_H
