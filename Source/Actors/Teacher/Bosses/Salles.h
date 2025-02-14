//
// Created by nslop on 25/09/2024.
//

#ifndef DPIBULLETHELLRE_SALLES_H
#define DPIBULLETHELLRE_SALLES_H


#include "../Boss.h"

class Salles : public Boss {

public:
    Salles(class Scene* scene);
    void OnUpdate(float deltaTime) override;


    void Start() override;

protected:

    bool Movement0() override;
    void Movement1() override;
    void Movement2() override;
    void Movement3() override;
    void Attack1() override;
    void Attack2() override;
    void Attack3() override;

private:

    std::string mProjectileSpritePath;
    std::string mProjectileDataPath;

    void DefineAtkTimers(float d, float d1, float d2);
};


#endif //DPIBULLETHELLRE_SALLES_H
