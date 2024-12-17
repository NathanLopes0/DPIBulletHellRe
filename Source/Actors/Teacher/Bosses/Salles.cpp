//
// Created by nslop on 25/09/2024.
//

#include "Salles.h"
#include "../../../Random.h"
#include "BossesProjectiles/BossProjectile.h"

Salles::Salles(Scene *scene)
               :Boss(scene) {


    //Define a velocidade de ataque de cada estado.
    // TODO 12.0 - Decidir e definir atkSpeed de cada estado (depois que pensar direitinho nos ataques)
    // TODO 12.0.1 - Isso vai ser decidido com testes, e é colocado em um vetor pra ser usado no future
    DefineAtkTimers(0.7f, 0.6f, 0.8f);

}

// TODO 14.0 - Verificar se podemos realocar alguma variavel para Bossfactory

void Salles::OnUpdate(float deltaTime) {

    DecreaseAtkTimer(deltaTime);
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


    // ----- PARAMETROS EM TEMPO DE EXECUÇÃO PARA ESTRATÉGIA 1 - ANGLEDATTACK ----- //
    auto randomCentralAngle = Random::GetIntRange(20, 140);
    auto projectiles = mAtkStrategies[0]->execute(0, 0, randomCentralAngle);


    ResetAtkTimer();

}

void Salles::Attack2() {

    // ----- PARAMETROS EM TEMPO DE EXECUÇÃO PARA ESTRATÉGIA 2 - ANGLEDATTACK ----- //
    auto randomCentralAngle = Random::GetIntRange(20, 140);
    auto homingChance = Random::GetFloatRange(0,1);
    auto projectiles = mAtkStrategies[1]->execute(0,0,randomCentralAngle);
    if(homingChance < 0.3) {
        for(auto& p : projectiles) {
            p->GetComponent<DrawAnimatedComponent>()->SetAnimation("Homing");
            p->insertBehavior<SlowDownBehavior>(1.3f, 0.5f);
            p->insertBehavior<HomingBehavior>(1.6f, 300.f);
        }
    }

    ResetAtkTimer();

}

void Salles::Attack3() {

    // ----- PARAMETROS EM TEMPO DE EXECUÇÃO PARA ESTRATÉGIA 3 - ANGLEDFILLEDATTACK ----- //
    auto randomCentralAngle = Random::GetIntRange(20, 140);
    float homingChance;
    auto projectiles = mAtkStrategies[2]->execute(0,0,0,randomCentralAngle);
    for (auto& p : projectiles) {
         homingChance = Random::GetFloatRange(0,1);
         if(homingChance < 0.55) {
            p->GetComponent<DrawAnimatedComponent>()->SetAnimation("Homing");
            p->insertBehavior<SlowDownBehavior>(0.8f, 0.5f);
            p->insertBehavior<HomingBehavior>(1.f, 250.f);
            p->insertBehavior<SlowDownBehavior>(2.2f, 0.5f);
            p->insertBehavior<HomingBehavior>(2.5f, 450.f);
        }
    }

    ResetAtkTimer();

}

void Salles::Start() {

    mFSMComponent->Start("StartState");

}

bool Salles::Movement0() {

    //Não coloquei nenhuma lógica aqui, pois o Boss só desce, e isso é fixado na sua criação
    //Botaria algum código aqui se houvesse alguma mudança de movimento apenas.
    return true;
}

void Salles::DefineAtkTimers(float stateOneTimer, float stateTwoTimer, float stateThreeTimer) {

    mStateAtkTimers["StateOne"] = stateOneTimer;
    mStateAtkTimers["StateTwo"] = stateTwoTimer;
    mStateAtkTimers["StateThree"] = stateThreeTimer;

}

// TODO 9.0 - Colocar isso na classe base Boss, pq vai ser sempre isso aqui.
void Salles::ResetAtkTimer() {

    mAtkTimer = mStateAtkTimers[mFSMComponent->GetState()->GetName()];

}


