//
// Created by gensh on 27/11/2025.
//

#include "AndreFactory.h"

#include <complex>

#include "../../Teachers/Bosses/Andre.h"
#include "../../../Actors/Teachers/BossFactory/BossProjectileFactory/AndreProjectile1Factory.h"
#include "../../../Attacks/AttackParameters/BaloonAttackParams.h"
#include "../../../Attacks/BaseStrategies/BaloonAttack.h"
#include "../../../Attacks/BaseStrategies/WaveAttack.h"
#include "BossProjectileFactory/AndreBaloonProjectileFactory.h"

AndreFactory::AndreFactory(Game *game)
    : IBossFactory(game)
{

}

std::unique_ptr<Boss> AndreFactory::InstantiateBoss(Scene* scene) {
    auto boss = std::make_unique<Andre>(scene);
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

    boss->AddProjectileFactory("Baloes", std::make_unique<AndreBaloonProjectileFactory>());

}

void AndreFactory::ConfigureAttacksAndFSM(Boss *boss) {

    auto fsm = boss->GetComponent<FSMComponent>();
    if (!fsm) { SDL_Log("ERRO CRÍTICO: Boss não tem FSMComponent!"); return; }

    ConfigureStateOne(boss, fsm);
    ConfigureStateTwo(boss, fsm);
    ConfigureStateThree(boss, fsm);
    ConfigureStateFinal(boss, fsm);

    boss->SetInitialState("StateOne");
}

void AndreFactory::ConfigureStateOne(Boss *boss, FSMComponent *fsm) {
    const std::string STATE_NAME = "StateOne";

    auto params = std::make_unique<BaloonAttackParams>();
    params->numProjectiles = 100;
    params->randomSpawn = true;
    params->centerOnPlayer = true;
    params->centerOnPlayerOffset = 800.f;
    params->side = BaloonAttackParams::side::Right;
    params->projectileSpeed = 630.f;

    auto spawner = boss->GetProjectileFactory("Baloes");


    boss->AddAttackPattern(STATE_NAME,
        std::make_unique<BaloonAttack>(spawner, boss),
        std::move(params),
        4.f);

    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
                                                            STATE_ONE_DURATION + 20,
                                                            "StateTwo");
    fsm->RegisterState(std::move(stateObj));

    boss->RegisterMovementStrategy(STATE_NAME, std::make_unique<GoToCenterStrategy>());

}

void AndreFactory::ConfigureStateTwo(Boss *boss, FSMComponent *fsm) {
    const std::string STATE_NAME = "StateTwo";

    auto params = std::make_unique<AttackParams>();
    params->numProjectiles = 20;
    params->projectileSpeed = 160.f;
    params->angle = 120.f;
    // TODO - mudar params.centralAngle no próprio Boss.
    params->creationSpeed = 0.15f;

    auto spawner = boss->GetProjectileFactory("Baloes");

    boss->AddAttackPattern(STATE_NAME,
        std::make_unique<WaveAttack>(spawner, boss),
        std::move(params),
        2.5f);

    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
                                                            STATE_TWO_DURATION,
                                                            "StateThree");
    fsm->RegisterState(std::move(stateObj));

    boss->RegisterMovementStrategy(STATE_NAME, std::make_unique<RandomWanderStrategy>(1.8f, 300.f));
}

void AndreFactory::ConfigureStateThree(Boss *boss, FSMComponent *fsm) {
    const std::string STATE_NAME = "StateThree";

    auto params = std::make_unique<AttackParams>();
    params->numProjectiles = 18;
    params->projectileSpeed = 200.f;
    params->angle = 180.f;
    params->centralAngle = 90.f;
    params->creationSpeed = 0.02f;
    auto spawner = boss->GetProjectileFactory("Baloes");

    boss->AddAttackPattern(STATE_NAME,
        std::make_unique<WaveAttack>(spawner, boss),
        std::move(params),
        0.9f);

    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
                                                            STATE_THREE_DURATION,
                                                            "");
    fsm->RegisterState(std::move(stateObj));

    boss->RegisterMovementStrategy(STATE_NAME, std::make_unique<GoToCenterStrategy>());
}

void AndreFactory::ConfigureStateFinal(Boss *boss, FSMComponent *fsm) {

    // TODO - código placeholder, igual ao do StateThree

    const std::string STATE_NAME = "StateFinal";

    auto params = std::make_unique<AttackParams>();
    params->numProjectiles = 40;
    params->projectileSpeed = 200.f;
    params->angle = 180.f;
    params->centralAngle = 90.f;
    params->creationSpeed = 0.02f;

    auto spawner = boss->GetProjectileFactory("Baloes");

    boss->AddAttackPattern(STATE_NAME,
        std::make_unique<WaveAttack>(spawner, boss),
        std::move(params),
        2.4f);

    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
                                                            STATE_FINAL_DURATION,
                                                            "StateOne");
    fsm->RegisterState(std::move(stateObj));

    boss->RegisterMovementStrategy(STATE_NAME, std::make_unique<GoToCenterStrategy>());
}






