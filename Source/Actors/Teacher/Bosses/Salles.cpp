//
// Created by nslop on 25/09/2024.
//

#include "Salles.h"
#include "BossesProjectiles/SallesBossProjectile.h"

Salles::Salles(Scene *scene, const std::string& spritePath, const std::string& dataPath,
               const std::string& projectileSpritePath, const std::string& projectileDataPath)
               :Boss(scene, spritePath, dataPath) {

    //Draw Component commands----------------------------------//
    //Coloquei a declaração de mDraw no boss, pois todos têm sprite, mas as animações coloco aqui.
    mDrawComponent->AddAnimation("Idle", {0});
    mDrawComponent->SetAnimation("Idle");
    //---------------------------------------------------------//.


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

// TODO 0.5 - Percebi que vou precisar modificar o jeito que trato os ataques
// TODO 0.5.1 - Criar uma nova BRANCH no git pra mexer com o Padrão de Projeto "Strategy"para os ataques.

void Salles::Attack1() {
    SDL_Log("Attacking1");

    float ATK1_PROJECTILES_fOWARDSPEED = 10.f;

    //vetor para manipular todos os projéteis de um único ataque
    std::vector<SallesBossProjectile*> Atk1Projectiles;


    // ------------- DECLARAÇÃO DE TODOS OS PROJÉTEIS DE CADA ATAQUE1 ------------------ //
    auto s1 = Atk1Projectiles.emplace_back(new SallesBossProjectile(mScene,this,mProjectileSpritePath,mProjectileDataPath));
    auto s2 = Atk1Projectiles.emplace_back(new SallesBossProjectile(mScene,this,mProjectileSpritePath,mProjectileDataPath));

    // ------------- DEfINIÇÃO DA VELOCIDADE DE TODOS OS PROJÉTEIS DE ATAQUE1 --------------- //
    for(auto projectile : Atk1Projectiles)
        projectile->SetFowardSpeed(ATK1_PROJECTILES_fOWARDSPEED);

    // ------------- DEfINIÇÃO DA DIREÇÃO DE CADA PROJÉTIL DE ATAQUE1 ---------------- //
    //isso significa setar a velocidade usando um vetor normalizado na direção desejada, e multiplicar
    //pela fowardSpeed definida acima.

    //TODO 1.9 - Como achar o vetor normalizado desejado? Nesse caso específico, tenho um ângulo de 30º que quero entre eles,
    //TODO 1.9 - e quero que eles estejam a 15º de uma reta abaixo do Boss.

    // TODO 1.7 - Primeiro, vou ver se consigo invocar os 2 projéteis usando só os vetores -1,-1 e 1,-1.
    s1->GetComponent<RigidBodyComponent>()->SetVelocity(Vector2(-1,-1) * s1->GetFowardSpeed());
    s1->GetComponent<RigidBodyComponent>()->SetVelocity(Vector2(1, -1) * s2->GetFowardSpeed());

    ResetAtkTimer();

    //OS ATAQUES NÃO ESTÃO APARECENDO POIS ELES SÃO DESALOCADOS ASSIM QUE SAEM DESSA fUNÇÃO OK? OK!

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

// TODO 4.0 - Colocar isso na classe base Boss, pq vai ser sempre isso aqui.
void Salles::ResetAtkTimer() {

    mAtkTimer = mStateAtkTimers[mFSMComponent->GetState()->GetName()];

}


