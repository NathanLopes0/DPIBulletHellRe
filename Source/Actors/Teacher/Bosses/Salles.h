//
// Created by nslop on 25/09/2024.
//

#ifndef DPIBULLETHELLRE_SALLES_H
#define DPIBULLETHELLRE_SALLES_H


#include "../Boss.h"

class Salles : public Boss {

public:
    Salles(Scene* scene);
    ~Salles() override = default;
    void OnUpdate(float deltaTime) override;

};


#endif //DPIBULLETHELLRE_SALLES_H
