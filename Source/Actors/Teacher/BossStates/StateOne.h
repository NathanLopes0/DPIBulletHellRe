//
// Created by nslop on 19/11/2024.
//

#ifndef DPIBULLETHELLRE_STATEONE_H
#define DPIBULLETHELLRE_STATEONE_H

#include "BossState.h"

class StateOne : public BossState {

public:
    StateOne(FSMComponent* stateMachine);
    void Start() override;
    void Update(float deltaTime) override;

    void HandleStateTransition(float stateTime) override;

};


#endif //DPIBULLETHELLRE_STATEONE_H
