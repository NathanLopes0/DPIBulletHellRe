//
// Created by nslop on 25/09/2024.
//

#include <string>
#include <memory>
#include "../Boss.h"
#include "Boss1Factory.h"
#include "../Bosses/Salles.h"
#include "../BossAttackStrategies/BaseStrategies/WaveAttack.h"
#include "../BossAttackStrategies/BaseStrategies/AngledAttack.h"
#include "../BossAttackStrategies/BaseStrategies/AngledFilledAttack.h"

Boss1Factory::Boss1Factory(Game *game) : BossFactory(game) {

}

Boss* Boss1Factory::CreateBoss(Scene* scene) {

    // TODO 32.5 - Dividir essa função em várias outras ("uma função não deve fazer mais de 3 coisas" - Little Lu)


    // ----------------  CONfIGURAÇÃO SPRITE BOSS E CRIAÇÃO DO BOSS, DRAW E COLLIDER ----------------//
    configureBossSprite();
    mBoss = new Salles(scene);
    auto* BossDrawComponent = new DrawAnimatedComponent(mBoss, spritePathing[0], dataPathing[0]);
    auto* CircleCollider = new CircleColliderComponent(mBoss, (float)BossDrawComponent->GetSpriteWidth() / 2.2f);
    // ---------------------------------------------------------------------------------------------//


    // ---------------------------- CONfIGURAÇÃO DOS PROJÉTEIS -------------------------------------//


    //mProjectileFactories.emplace_back(new Boss1Projectile1Factory());
    // ---------------------------------------------------------------------------------------------//

    // --------- Configura animações, DrawComponent inicializado lá em cima -------- //
    configureBaseDrawComponent(BossDrawComponent);

    // --------- Cria estratégias de ataque -------- //
    mProjectileSpawner = new Boss1Projectile1Factory();
    configureStrategies();


    // ---------------------------- INJEÇÃO DE COMPONENTES NO BOSS ------------------------------ //

    mBoss->insertComponents(BossDrawComponent, CircleCollider);
    mBoss->insertAttackStrategies(mAtkStrategies);

    // ------------------------------------------------------------------------------------------- //




    //      ----------DEFINIÇÃO DE POSIÇÃO E VELOCIDADE INICIAL, E ATIVAÇÃO DO BOSS----------      //
    mBoss->SetState(ActorState::Active);
    mBoss->GetComponent<DrawAnimatedComponent>()->SetIsVisible(true);

    auto midWidth = mBoss->GetWindowsWidth() / 2;
    auto spriteHeight = mBoss->GetSpriteHeight();

    mBoss->SetPosition(Vector2((float)midWidth, -1.f * (float)spriteHeight));
    mBoss->GetComponent<RigidBodyComponent>()->SetVelocity(Vector2(0, 50));
    //      ------------------- -- ------- - ---------- -------- - -------- -- ---------------      //


    return mBoss;
}

void Boss1Factory::configureStrategies() {

    //talvez criar função defineStrategy1, defineStrategy2, defineStrategy3, para ficar melhor de mudar no futuro.
    std::vector<AttackStrategy*> Attacks;
    //----- PRIMEIRA ESTRATÉGIA

    Attacks.emplace_back(new AngledAttack(mProjectileSpawner, mBoss, 250.f, 10, 90));

    //----- SEGUNDA ESTRATÉGIA

    Attacks.emplace_back(new AngledAttack(mProjectileSpawner, mBoss, 250.f, 15, 90));

    //----- TERCEIRA ESTRATÉGIA

    Attacks.emplace_back(new AngledFilledAttack(mProjectileSpawner, mBoss, 3,
                                                240.f, 20, 90));


    mAtkStrategies.insert(mAtkStrategies.end(),
                          std::make_move_iterator(Attacks.begin()),
                          std::make_move_iterator(Attacks.end()));

    //pode ser interessante ter um vetor de ints mostrando a posição em mAtkStrategies que começa e termina as estratégias da X

}

void Boss1Factory::configureBossSprite() {

    spritePathing.emplace_back("../Assets/Teachers/DPIBHSalles.png");
    dataPathing.emplace_back("../Assets/Teachers/DPIBHSalles.json");

}
