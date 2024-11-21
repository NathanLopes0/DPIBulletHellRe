//
// Created by nslop on 15/10/2024.
//

#ifndef DPIBULLETHELLRE_STARTSTATE_H
#define DPIBULLETHELLRE_STARTSTATE_H

#include "BossState.h"

class StartState : public BossState {

public:
    explicit StartState(FSMComponent* stateMachine);


    void Start() override;
    void Update(float deltaTime) override;
    void HandleStateTransition(float stateTime) override;

};


#endif //DPIBULLETHELLRE_STARTSTATE_H
