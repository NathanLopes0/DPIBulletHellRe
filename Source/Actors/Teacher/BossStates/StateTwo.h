//
// Created by nslop on 12/12/2024.
//

#ifndef DPIBULLETHELLRE_STATETWO_H
#define DPIBULLETHELLRE_STATETWO_H


#include "BossState.h"

class StateTwo : public BossState {

public:
    StateTwo(FSMComponent* stateMachine);
    void Start() override;
    void Update(float deltaTime) override;

    void HandleStateTransition(float stateTime) override;


};


#endif //DPIBULLETHELLRE_STATETWO_H
