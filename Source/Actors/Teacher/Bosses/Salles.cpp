//
// Created by nslop on 25/09/2024.
//

#include "Salles.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../../Components/AIComponents/FSMComponent.h"

Salles::Salles(Scene *scene) : Boss(scene)
{
    // --- MONTAGEM DOS COMPONENTES VISUAIS E FÍSICOS --- //
    // A posse dos componentes é entregue ao sistema do Actor.
    auto drawComp = AddComponent<DrawAnimatedComponent>("../Assets/Teachers/DPIBHSalles.png", "../Assets/Teachers/DPIBHSalles.json");
    drawComp->AddAnimation("Idle", {0});
    drawComp->SetAnimation("Idle");

    // Inicialização do colisor
    const float colliderRadius = static_cast<float>(drawComp->GetSpriteWidth()) / 2.2f;
    AddComponent<CircleColliderComponent>(colliderRadius);

    // --- INICIALIZAÇÃO DA LÓGICA INTERNA --- //
    InitializeFSM();
    InitializeAttackStrategies();
}

// TODO 4.0 - Verificar se podemos realocar alguma variavel para Bossfactory
// TODO 3.0 - Montar função de ataque e movimento do primeiro estado do Salles

void Salles::OnUpdate(float deltaTime) {

    DecreaseAtkTimer(deltaTime);

}

//                    --------------STATE FUNCTIONS----------------                    //


void Salles::Movement1() {

}

void Salles::Movement2() {

}

void Salles::Movement3() {

}

bool Salles::Movement0() {

    //Não coloquei nenhuma lógica aqui, pois o Boss só desce, e isso é fixado na sua criação
    //Botaria algum código aqui se houvesse alguma mudança de movimento apenas.
    return true;
}


void Salles::ResetAtkTimer() {

    mAtkTimer = mStateAtkTimers[GetComponent<FSMComponent>()->GetState()->GetName()];

}


