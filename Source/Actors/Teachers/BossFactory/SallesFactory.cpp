
#include "SallesFactory.h"
#include "../../Teachers/Bosses/Salles.h"
#include "../../../Attacks/BaseStrategies/AngledAttack.h"
#include "../../../Actors/Teachers/BossFactory/BossProjectileFactory/SallesProjectile1Factory.h"
#include "../../../Actors/Teachers/BossFactory/BossProjectileFactory/SallesDoubleListProjectileFactory.h"
#include "../../../Attacks/BaseStrategies/LaserAttack.h"


SallesFactory::SallesFactory(Game* game)
    : IBossFactory(game)
{
}

std::unique_ptr<Boss> SallesFactory::InstantiateBoss(Scene* scene) {
    return std::make_unique<Salles>(scene);
}

void SallesFactory::ConfigureComponents(Boss* boss) {

    // ----- DRAW COMPONENT ----- //
    auto drawComp = boss->AddComponent<DrawAnimatedComponent>("../Assets/Teachers/DPIBHSalles.png",
                                                                                "../Assets/Teachers/DPIBHSalles.json");
    drawComp->AddAnimation("Idle", {0});
    drawComp->SetAnimation("Idle");


    // ----- COLLIDER COMPONENT ----- //
    const float colliderRadius = static_cast<float>(drawComp->GetSpriteWidth()) / 2.2f;
    auto collider = boss->AddComponent<CircleColliderComponent>(colliderRadius);
    collider->SetTag(ColliderTag::Boss);

    boss->AddProjectileFactory("Capivara",std::make_unique<SallesProjectile1Factory>());
    boss->AddProjectileFactory("ListaDuplamente", std::make_unique<SallesDoubleListProjectileFactory>());

}


void SallesFactory::ConfigureAttacksAndFSM(Boss* boss) {


    auto fsm = boss->GetComponent<FSMComponent>();
    if (!fsm) { SDL_Log("ERRO CRÍTICO: Boss não tem FSMComponent!"); return; }

    ConfigureStateOne(boss, fsm);
    ConfigureStateTwo(boss, fsm);
    ConfigureStateThree(boss, fsm);
    ConfigureStateFinal(boss, fsm);

    boss->SetInitialState("StateOne");
}

void SallesFactory::ConfigureStateOne(Boss* boss, FSMComponent* fsm)
{

    const std::string STATE_NAME = "StateOne";

    // 1. Configura os parâmetros fixos
    auto params = std::make_unique<AttackParams>();
    params->projectileSpeed = 230.0f;
    params->angle = 30.f;

    auto spawner = boss->GetProjectileFactory("ListaDuplamente");

    boss->AddAttackPattern(STATE_NAME,
            std::make_unique<LaserAttack>(spawner, boss), // Strategy
            std::move(params),                                        // Params
            4.f//,                                          // Cooldown
/*            [](Projectile* p, int index) {                 // Configurator Lambda
                if (Random::GetFloatRange(0.0f, 1.0f) < 0.2f) {
                    p->insertBehavior<HomingBehavior>(1.2f, 280.f);
                    p->GetComponent<DrawAnimatedComponent>()->SetAnimation("Homing");

                }
            }*/
        );

    // Cria o objeto que representa o estado que terá esse ataque
    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
                                                      /*duration*/ STATE_ONE_DURATION,
                                                      /*nextState*/ "StateTwo");
    fsm->RegisterState(std::move(stateObj));

    boss->RegisterMovementStrategy(STATE_NAME, std::make_unique<GoToCenterStrategy>());//(3.f, 120.f));


}
void SallesFactory::ConfigureStateTwo(Boss* boss, FSMComponent* fsm)
{

    const std::string STATE_NAME = "StateTwo";

    // 1. Configura os parâmetros para este estado
    auto params = std::make_unique<AttackParams>();
    params->numProjectiles = 3;
    params->projectileSpeed = 230.0f;
    params->angle = 40.f;

    auto spawner = boss->GetProjectileFactory("Capivara");

    boss->AddAttackPattern(STATE_NAME,
        std::make_unique<AngledAttack>(spawner, boss),
        std::move(params),
        1.f,
        [](Projectile* p, int index) {
            if (Random::GetFloatRange(0.0f, 1.0f) < 0.3f) {
                p->insertBehavior<HomingBehavior>(1.8f, 0.0f);
                p->GetComponent<DrawAnimatedComponent>()->SetAnimation("Homing");
            }
        }
    );

    // 2. Cria o objeto que representa o estado desse ataque
    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
                                                      /*duration*/ STATE_TWO_DURATION,
                                                      /*nextState*/ "StateThree");
    fsm->RegisterState(std::move(stateObj));

    boss->RegisterMovementStrategy(STATE_NAME,
        std::make_unique<RandomWanderStrategy>(6.0f, 300.0f));

}
void SallesFactory::ConfigureStateThree(Boss *boss, FSMComponent *fsm) {
    const std::string STATE_NAME = "StateThree";

    // 1 . Configura os padrões fixos do estado três
    auto params = std::make_unique<AttackParams>();
    params->numProjectiles = 4;
    params->projectileSpeed = 260.0f;
    params->angle = 60.f;

    auto spawner = boss->GetProjectileFactory("Capivara");

    boss->AddAttackPattern(STATE_NAME,
        std::make_unique<AngledAttack>(spawner, boss),
        std::move(params),
        0.9f,
        [](Projectile* p, int index) {
            if (Random::GetFloatRange(0.0f, 1.0f) < 0.5f) {
                p->insertBehavior<HomingBehavior>(1.8f, 0.0f);
                p->GetComponent<DrawAnimatedComponent>()->SetAnimation("Homing");
            }
        }
    );

    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
                                                  /*duration*/ STATE_THREE_DURATION,
                                                  /*nextState*/ "");

    fsm->RegisterState(std::move(stateObj));

    boss->RegisterMovementStrategy(STATE_NAME,
        std::make_unique<RandomWanderStrategy>(4.f, 300.f));

}
void SallesFactory::ConfigureStateFinal(Boss *boss, FSMComponent *fsm) {
    const std::string STATE_NAME = "StateFinal";

    // 1. Padrões fixos do estado Final
    // -- Ataque 1 --
    auto paramsFast = std::make_unique<AttackParams>();
    paramsFast->numProjectiles = 2;
    paramsFast->projectileSpeed = 280.0f;
    paramsFast->angle = 40.f;

    auto spawner = boss->GetProjectileFactory("Capivara");

    boss->AddAttackPattern(STATE_NAME,
        std::make_unique<AngledAttack>(spawner, boss),
        std::move(paramsFast),
        0.8f,
        [](Projectile* p, int i) {
            auto chance = Random::GetFloatRange(0.0f, 1.0f);
            if (chance < 0.1) {
                p->insertBehavior<HomingBehavior>(1.2f, 0.0f);
                p->GetComponent<DrawAnimatedComponent>()->SetAnimation("Homing");
            }
        });

    // -- Ataque 2 --
    auto paramsSlow = std::make_unique<AttackParams>();
    paramsSlow->numProjectiles = 3;
    paramsSlow->projectileSpeed = 200.0f;
    paramsSlow->angle = 80.f;

    boss->AddAttackPattern(STATE_NAME,
        std::make_unique<AngledAttack>(spawner, boss),
        std::move(paramsSlow),
        1.f,
        [](Projectile* p, int i) {
            auto chance = Random::GetFloatRange(0.0f, 1.0f);
            if (chance < 0.4) {
                p->insertBehavior<HomingBehavior>(1.6f, 0.0f);
                p->GetComponent<DrawAnimatedComponent>()->SetAnimation("Homing");
            }
        });

    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
                                                    STATE_FINAL_DURATION,
                                                        "StateOne");
    fsm->RegisterState(std::move(stateObj));

    boss->RegisterMovementStrategy(STATE_NAME,
        std::make_unique<GoToCenterStrategy>());

}


