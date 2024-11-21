//
// Created by nslop on 25/09/2024.
//

#include "Salles.h"

Salles::Salles(Scene *scene, const std::string& spritePath, const std::string& dataPath) : Boss(scene) {

    //Draw Component commands----------------------------------//
    //Coloquei a declaração de Draw no Boss Específico, pois eles podem ter animações diferentes.
    mDrawComponent = new DrawAnimatedComponent(this, spritePath, dataPath);
    mDrawComponent->AddAnimation("Idle", {0});
    mDrawComponent->SetAnimation("Idle");
    //---------------------------------------------------------//


}
// TODO 2.0 - colocar HandleStateTransition do StartState para realmente mudar para StateOne, verificar se ta tudo certo
// TODO 4.0 - Verificar se podemos realocar alguma variavel para Bossfactory
// TODO 3.0 - Montar função de ataque e movimento do primeiro estado do Salles

void Salles::OnUpdate(float deltaTime) {

    StateActions();

}

//                    --------------STATE FUNCTIONS----------------                    //


void Salles::Movement1() {

}

void Salles::Movement2() {

}

void Salles::Movement3() {

}

void Salles::Attack1() {

}

void Salles::Attack2() {

}

void Salles::Attack3() {

}

void Salles::Start() {

    mFSMComponent->Start("StartState");

}

bool Salles::Movement0() {

    //Não coloquei nenhuma lógica aqui, pois o Boss só desce, e isso é fixado na sua criação
    //Botaria algum código aqui se houvesse alguma mudança de movimento apenas.
    return true;
}
