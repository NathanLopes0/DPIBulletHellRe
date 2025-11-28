//
// Created by gensh on 27/11/2025.
//

#include "AndreFactory.h"
#include "../../Teachers/Bosses/Andre.h"
#include "../../../Actors/Teachers/BossFactory/BossProjectileFactory/AndreProjectile1Factory.h"
#include "../../../Attacks/BaseStrategies/WaveAttack.h"

AndreFactory::AndreFactory(Game *game)
    : IBossFactory(game)
{

}

std::unique_ptr<Boss> AndreFactory::InstantiateBoss(Scene* scene) {
    auto boss = std::make_unique<Andre>(scene);
    boss->SetScale(2.f); // TODO - Fazer uma sprite com o tamanho certo ao invés de mudar a escala em código
    return boss;
}

void AndreFactory::ConfigureComponents(Boss *boss) {

    // ----- DRAW COMPONENT ----- //
    auto drawComp = boss->AddComponent<DrawAnimatedComponent>("../Assets/Teachers/DPIBHAndre.png",
                                                                               "../Assets/Teachers/DPIBHAndre.json");
    drawComp->AddAnimation("Idle", {0});
    drawComp->SetAnimation("Idle");

    // ----- COLLIDER COMPONENT ----- //
    const float colliderRadius = static_cast<float>(drawComp->GetSpriteWidth()) / 2.2f;
    auto collider = boss->AddComponent<CircleColliderComponent>(colliderRadius);
    collider->SetTag(ColliderTag::Boss);

    boss->SetProjectileFactory(std::make_unique<AndreProjectile1Factory>());

}

void AndreFactory::ConfigureAttacksAndFSM(Boss *boss) {

    auto fsm = boss->GetComponent<FSMComponent>();
    if (!fsm) { SDL_Log("ERRO CRÍTICO: Boss não tem FSMComponent!"); return; }

    // Pega a Fábrica de Projéteis
    auto spawner = boss->GetProjectileFactory();
    if (!spawner) { SDL_Log("ERRO CRÍTICO: Boss não tem ProjectileFactory!"); return; }

    ConfigureStateOne(boss, fsm, spawner);
    ConfigureStateTwo(boss, fsm, spawner);
    ConfigureStateThree(boss, fsm, spawner);
    ConfigureStateFinal(boss, fsm, spawner);

    boss->SetInitialState("StateOne");
}

void AndreFactory::ConfigureStateOne(Boss *boss, FSMComponent *fsm, ProjectileFactory *spawner) {
    const std::string STATE_NAME = "StateOne";

    AttackParams params;
    params.numProjectiles = 18;
    params.projectileSpeed = 120.f;
    params.angle = 180.f;
    // TODO - mudar params.centralAngle no próprio Boss.
    params.creationSpeed = 0.08f;

    boss->AddAttackPattern(STATE_NAME,
        std::make_unique<WaveAttack>(spawner, boss),
        params,
        3.f);

    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
                                                            STATE_ONE_DURATION,
                                                            "StateTwo");
    fsm->RegisterState(std::move(stateObj));

    boss->RegisterMovementStrategy(STATE_NAME, std::make_unique<GoToCenterStrategy>());

}

void AndreFactory::ConfigureStateTwo(Boss *boss, FSMComponent *fsm, ProjectileFactory *spawner) {
    const std::string STATE_NAME = "StateTwo";

    AttackParams params;
    params.numProjectiles = 20;
    params.projectileSpeed = 160.f;
    params.angle = 120.f;
    // TODO - mudar params.centralAngle no próprio Boss.
    params.creationSpeed = 0.15f;

    boss->AddAttackPattern(STATE_NAME,
        std::make_unique<WaveAttack>(spawner, boss),
        params,
        2.5f);

    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
                                                            STATE_TWO_DURATION,
                                                            "StateThree");
    fsm->RegisterState(std::move(stateObj));

    boss->RegisterMovementStrategy(STATE_NAME, std::make_unique<RandomWanderStrategy>(1.8f, 300.f));
}

void AndreFactory::ConfigureStateThree(Boss *boss, FSMComponent *fsm, ProjectileFactory *spawner) {
    const std::string STATE_NAME = "StateThree";

    AttackParams params;
    params.numProjectiles = 18;
    params.projectileSpeed = 200.f;
    params.angle = 180.f;
    params.centralAngle = 90.f;
    params.creationSpeed = 0.02f;

    boss->AddAttackPattern(STATE_NAME,
        std::make_unique<WaveAttack>(spawner, boss),
        params,
        0.9f);

    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
                                                            STATE_THREE_DURATION,
                                                            "");
    fsm->RegisterState(std::move(stateObj));

    boss->RegisterMovementStrategy(STATE_NAME, std::make_unique<GoToCenterStrategy>());
}

void AndreFactory::ConfigureStateFinal(Boss *boss, FSMComponent *fsm, ProjectileFactory *spawner) {

    // TODO - código placeholder, igual ao do StateThree

    const std::string STATE_NAME = "StateFinal";

    AttackParams params;
    params.numProjectiles = 40;
    params.projectileSpeed = 200.f;
    params.angle = 180.f;
    params.centralAngle = 90.f;
    params.creationSpeed = 0.02f;

    boss->AddAttackPattern(STATE_NAME,
        std::make_unique<WaveAttack>(spawner, boss),
        params,
        2.4f);

    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
                                                            STATE_FINAL_DURATION,
                                                            "StateOne");
    fsm->RegisterState(std::move(stateObj));

    boss->RegisterMovementStrategy(STATE_NAME, std::make_unique<GoToCenterStrategy>());
}






