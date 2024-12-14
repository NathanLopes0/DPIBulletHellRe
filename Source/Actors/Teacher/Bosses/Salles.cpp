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
    DefineAtkTimers(0.5f, 1.f, 1.f);

}

// TODO 14.0 - Verificar se podemos realocar alguma variavel para Bossfactory
// TODO 13.0 - Montar função de ataque e movimento do primeiro estado do Salles

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

    //SDL_Log("Attacking1");

    // ---------- ESBOÇO DO CÓDIGO USANDO VETOR DE INTEIROS COM INTERVALOS DE ESTRATEGIAS ------------//
    /*
     * int i = 0;
     *  for (;i < intervaloStrat[0]; i++)
     *      mAtkStrategies[i]->execute();
     *
     *  i começaria de 0 por ser o ataque 1, e nos outros ataques?
     *  ataque 2 => int i = intervaloStrat[0];
     *  ataque 3 => int i = intervaloStrat[1] + intervaloStrat[0];
     *
     *
     */

    // ----- PARAMETROS EM TEMPO DE EXECUÇÃO PARA ESTRATÉGIA 1 - ANGLEDATTACK ----- //

    auto randomCentralAngle = Random::GetIntRange(20, 140);

    auto projectiles = mAtkStrategies[0]->execute(0, 0, randomCentralAngle);





    ResetAtkTimer();

}

void Salles::Attack2() {

    SDL_Log("Attacking 2");
    auto p1 = mAtkStrategies[1]->execute();
    auto p2 = mAtkStrategies[2]->execute();

    ResetAtkTimer();

}

void Salles::Attack3() {

    SDL_Log("Attacking 3");
    auto p1 = mAtkStrategies[3]->execute();

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


