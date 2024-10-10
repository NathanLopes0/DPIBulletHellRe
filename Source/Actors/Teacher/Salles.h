//
// Created by nslop on 25/09/2024.
//

#ifndef DPIBULLETHELLRE_SALLES_H
#define DPIBULLETHELLRE_SALLES_H


#include "Boss.h"

class Salles : public Boss {

public:
    Salles(class Scene* scene, const std::string& spritePath, const std::string& dataPath);
    void OnUpdate(float deltaTime) override;

    void Movement1() override;
    void Movement2() override;
    void Movement3() override;
    void Attack1() override;
    void Attack2() override;
    void Attack3() override;

    void SetState(const std::string& stateName) { mFSMComponent->SetState(stateName); }
    void Start();
private:



};


#endif //DPIBULLETHELLRE_SALLES_H
