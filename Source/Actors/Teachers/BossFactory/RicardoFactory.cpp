//
// Created by gensh on 26/11/2025.
//

#include "RicardoFactory.h"

#include "../BossAttackState.h"
#include "../../../Attacks/Behaviors.h"
#include "../../../Attacks/BaseStrategies/CircleSpreadAttack.h"
#include "../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Movements/MovementStrategies.h"
#include "../Bosses/Ricardo.h"
#include "BossProjectileFactory/RicardoProjectile1Factory.h"
#include "../../../Attacks/PathShapes.h"

RicardoFactory::RicardoFactory(Game *game)
    : IBossFactory(game)
{
}

std::unique_ptr<Boss> RicardoFactory::InstantiateBoss(Scene *scene) {
    return std::make_unique<Ricardo>(scene);
}

void RicardoFactory::ConfigureComponents(Boss *boss) {

    // ----- DRAW COMPONENT ----- //
    auto drawComp = boss->AddComponent<DrawAnimatedComponent>("../Assets/Teachers/DPIBHRicardo.png",
        "../Assets/Teachers/DPIBHRicardo.json");
    drawComp->AddAnimation("Idle", {0});
    drawComp->SetAnimation("Idle");

    // ----- COLLIDER COMPONENT ----- //
    const float colliderRadius = static_cast<float>(drawComp->GetSpriteWidth()) / 2.2f;
    const auto collider = boss->AddComponent<CircleColliderComponent>(colliderRadius);
    collider->SetTag(ColliderTag::Boss);

    boss->AddProjectileFactory("Arduino", std::make_unique<RicardoProjectile1Factory>());
}

void RicardoFactory::ConfigureAttacksAndFSM(Boss *boss) {
    auto fsm = boss->GetComponent<FSMComponent>();
    if (!fsm) {
        SDL_Log("ERRO CRÍTICO: Boss não tem FSMComponent");
        return;
    }


    ConfigureStateOne(boss, fsm);
    ConfigureStateTwo(boss, fsm);
    ConfigureStateThree(boss, fsm);
    ConfigureStateFinal(boss, fsm);

    boss->SetInitialState("StateOne");
}

void RicardoFactory::ConfigureStateOne(Boss *boss, FSMComponent *fsm) {

    const std::string STATE_NAME = "StateOne";

    // 1. Configurar parâmetros fixos
    auto params = std::make_unique<AttackParams>();
    params->numProjectiles = 36;
    params->projectileSpeed = 180.f;

    auto spawner = boss->GetProjectileFactory("Arduino");

    boss->AddAttackPattern(STATE_NAME,
        std::make_unique<CircleSpreadAttack>(spawner, boss),
        std::move(params),
        2.4f
        );

    // Criar objeto que representa o estado que terá esse ataque
    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
                                                        STATE_ONE_DURATION,
                                                        "StateTwo");
    fsm->RegisterState(std::move(stateObj));

    boss->RegisterMovementStrategy(STATE_NAME, std::make_unique<RandomWanderStrategy>(4.f, 100.f));

}

void RicardoFactory::ConfigureStateTwo(Boss *boss, FSMComponent *fsm) {
    const std::string STATE_NAME = "StateTwo";

    // 1. Configurar parâmetros fixos
    auto params = std::make_unique<AttackParams>();
    params->numProjectiles = 36;
    params->projectileSpeed = 180.f;

    auto spawner = boss->GetProjectileFactory("Arduino");

    boss->AddAttackPattern(STATE_NAME,
        std::make_unique<CircleSpreadAttack>(spawner, boss),
        std::move(params),
        2.4f,
        [](Projectile* p, int index) {
            p->insertModifier<SlowDownBehavior>(1.3f, 0.8f);
            // Era HomingBehavior(1.5f, 180.f). Atencao a ORDEM: Homing e (atraso,
            // velocidade); Path e (forma, velocidade, atraso, mira).
            p->insertMotion<PathBehavior>(PathShapes::Reta(), 180.f, 1.5f, Mira(Mira::MirarNoJogador));
        }
        );

    // Criar objeto que representa o estado que terá esse ataque
    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
                                                        STATE_TWO_DURATION,
                                                        "StateThree");
    fsm->RegisterState(std::move(stateObj));

    boss->RegisterMovementStrategy(STATE_NAME, std::make_unique<RandomWanderStrategy>(3.f, 160.f));
}

void RicardoFactory::ConfigureStateThree(Boss *boss, FSMComponent *fsm) {

    const std::string STATE_NAME = "StateThree";

    auto params = std::make_unique<AttackParams>();
    params->numProjectiles = 18;
    params->projectileSpeed = 180.f;

    auto spawner = boss->GetProjectileFactory("Arduino");

    boss->AddAttackPattern(STATE_NAME,
    std::make_unique<CircleSpreadAttack>(spawner, boss),
    std::move(params),
    2.4f,
    [](Projectile* p, const int index) {
        if (index % 2 == 0)
            p->insertMotion<PathBehavior>(PathShapes::Reta(), 200.f, 1.f, Mira(Mira::MirarNoJogador));
    }
    );

    // Criar objeto que representa o estado que terá esse ataque
    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
                                                        STATE_THREE_DURATION,
                                                        "");
    fsm->RegisterState(std::move(stateObj));

    boss->RegisterMovementStrategy(STATE_NAME, std::make_unique<RandomWanderStrategy>(2.f, 180.f));
}

void RicardoFactory::ConfigureStateFinal(Boss *boss, FSMComponent *fsm) {
    const std::string STATE_NAME = "StateFinal";

    auto paramsFast = std::make_unique<AttackParams>();
    paramsFast->numProjectiles = 18;
    paramsFast->projectileSpeed = 120.f;

    auto spawner = boss->GetProjectileFactory("Arduino");

    boss->AddAttackPattern(STATE_NAME,
        std::make_unique<CircleSpreadAttack>(spawner, boss),
        std::move(paramsFast),
        1.4f);

    auto paramsSlow = std::make_unique<AttackParams>();
    paramsSlow->numProjectiles = 9;
    paramsSlow->projectileSpeed = 240.f;

    boss->AddAttackPattern(STATE_NAME,
        std::make_unique<CircleSpreadAttack>(spawner, boss),
        std::move(paramsSlow),
        2.8f,
        [](Projectile* p, const int index) {
            p->insertModifier<SlowDownBehavior>(0.8f, 0.8f);
            p->insertMotion<PathBehavior>(PathShapes::Reta(), 240.f, 1.f, Mira(Mira::MirarNoJogador));
        });

    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
        STATE_FINAL_DURATION,
        "StateOne");

    fsm->RegisterState(std::move(stateObj));

    boss->RegisterMovementStrategy(STATE_NAME,
        std::make_unique<GoToCenterStrategy>());

}


