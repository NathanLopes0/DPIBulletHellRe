//
// Created by nslop on 15/12/2024.
//

#ifndef DPIBULLETHELLRE_STATETHREE_H
#define DPIBULLETHELLRE_STATETHREE_H


#include "BossState.h"

class StateThree : public BossState {

public:
    StateThree(FSMComponent* stateMachine);
    void Start() override;
    void Update(float deltaTime) override;

    void HandleStateTransition(float stateTime) override;

};


#endif //DPIBULLETHELLRE_STATETHREE_H
