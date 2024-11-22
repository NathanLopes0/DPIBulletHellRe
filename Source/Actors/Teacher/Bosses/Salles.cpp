//
// Created by nslop on 25/09/2024.
//

#include "Salles.h"
#include "BossesProjectiles/SallesBossProjectile.h"

Salles::Salles(Scene *scene, const std::string& spritePath, const std::string& dataPath,
               const std::string& projectileSpritePath, const std::string& projectileDataPath) : Boss(scene) {

    //Draw Component commands----------------------------------//
    //Coloquei a declaração de Draw no Boss Específico, pois eles podem ter animações diferentes.
    mDrawComponent = new DrawAnimatedComponent(this, spritePath, dataPath);
    mDrawComponent->AddAnimation("Idle", {0});
    mDrawComponent->SetAnimation("Idle");
    //---------------------------------------------------------//.


    // a divisao por 2.2f é melhor do que a divisão por 2, pois a 2 fica grande demais. a 2.2 pega fora dos braços, mas por pouco.
    //SDL_Log("%d, %d", mDrawComponent->GetSpriteWidth(), mDrawComponent->GetSpriteHeight());
    mColliderComponent = new CircleColliderComponent(this, (float)mDrawComponent->GetSpriteWidth() / 2.2f);


    //String com o Path da sprite e data dos Projéteis, para serem criados nas funções de Ataque.
    mProjectileSpritePath = projectileSpritePath;
    mProjectileDataPath = projectileDataPath;

    //Define a velocidade de ataque de cada estado.
    // TODO 7.0 - Decidir e definir atkSpeed de cada estado (depois que pensar direitinho nos ataques)
    // TODO 7.0.1 - Isso vai ser decidido com testes, e é colocado em um vetor pra ser usado no future
    DefineAtkTimers(1.5f, 1.f, 1.f);


}

// TODO 4.0 - Verificar se podemos realocar alguma variavel para Bossfactory
// TODO 3.0 - Montar função de ataque e movimento do primeiro estado do Salles

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
    SDL_Log("Attacking1");
    std::vector<SallesBossProjectile*> Atk1Projectiles;

    ResetAtkTimer();

    //auto s1 = Atk1Projectiles.emplace_back(new SallesBossProjectile(mScene,this,mProjectileSpritePath,mProjectileDataPath));
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

void Salles::DefineAtkTimers(float stateOneTimer, float stateTwoTimer, float stateThreeTimer) {

    mStateAtkTimers["StateOne"] = stateOneTimer;
    mStateAtkTimers["StateTwo"] = stateTwoTimer;
    mStateAtkTimers["StateThree"] = stateThreeTimer;

}

void Salles::ResetAtkTimer() {

    mAtkTimer = mStateAtkTimers[mFSMComponent->GetState()->GetName()];

}


