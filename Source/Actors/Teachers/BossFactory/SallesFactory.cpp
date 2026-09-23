
#include "SallesFactory.h"
#include "../../Teachers/Bosses/Salles.h"
#include "../../../Attacks/BaseStrategies/AngledAttack.h"
#include "../../../Actors/Teachers/BossFactory/BossProjectileFactory/SallesProjectile1Factory.h"
#include "../../../Actors/Teachers/BossFactory/BossProjectileFactory/SallesDoubleListProjectileFactory.h"
#include "../../../Attacks/BaseStrategies/LaserAttack.h"
#include "../../../Attacks/PathShapes.h"

// =============================================================================
// MODO DE TESTE DO PATHING
//
// Apenas os CONFIGURATORS foram trocados. Os blocos de params (numProjectiles,
// projectileSpeed, angle) estao INTACTOS - seu balanceamento nao foi tocado.
//
// A velocidade do caminho e controlada pelo 3o argumento do PathBehavior, e nao
// pelo projectileSpeed: o projetil nasce na SUA velocidade e o caminho e
// percorrido numa velocidade legivel. A 340-400 px/s um laco vira um risco.
//
// Para reverter: troque cada insertMotion<PathBehavior> de volta pelo
// PathBehavior mirado no jogador comentado logo acima dele (equivalente
// exato do antigo HomingBehavior, removido na fase 3 - provado em
// tests/test_motion_contract.cpp).
// =============================================================================


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
    params->numProjectiles = 3;
    params->projectileSpeed = 340.0f;
    params->angle = 40.f;

    auto spawner = boss->GetProjectileFactory("Capivara");

    boss->AddAttackPattern(STATE_NAME,
            std::make_unique<AngledAttack>(spawner, boss), // Strategy
            std::move(params),                                        // Params
            .8f,                                          // Cooldown
            [](Projectile* p, int index){                 // Configurator Lambda
                {   // ORIGINAL: if (rand<0.2) insertMotion<PathBehavior>(PathShapes::Reta(), 280.f, 1.2f, Mira(Mira::MirarNoJogador));
                    // TESTE - ZIGZAG: quinas exatas, legiveis. 6 pernas de 85px.
                    p->insertMotion<PathBehavior>(PathShapes::Zigzag(85.f, 50.f, 6), 200.f);

                }
            }
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
    params->numProjectiles = 4;
    params->projectileSpeed = 360.0f;
    params->angle = 40.f;

    auto spawner = boss->GetProjectileFactory("Capivara");

    boss->AddAttackPattern(STATE_NAME,
        std::make_unique<AngledAttack>(spawner, boss),
        std::move(params),
        1.f,
        [](Projectile* p, int index) {
            {   // ORIGINAL: if (rand<0.3) insertMotion<PathBehavior>(PathShapes::Reta(), 0.f, 1.8f, Mira(Mira::MirarNoJogador));
                // TESTE - ARCO alternado: as barrigas se cruzam no meio do voo.
                const float lat = (index % 2 == 0) ? 110.f : -110.f;
                p->insertMotion<PathBehavior>(PathShapes::Arc(500.f, lat, 10), 190.f);
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
    params->numProjectiles = 8;
    params->projectileSpeed = 600.0f;
    params->angle = 80.f;

    auto spawner = boss->GetProjectileFactory("Capivara");

    boss->AddAttackPattern(STATE_NAME,
        std::make_unique<AngledAttack>(spawner, boss),
        std::move(params),
        0.8f,
        [](Projectile* p, int index) {
            {   // ORIGINAL: if (rand<0.5) insertMotion<PathBehavior>(PathShapes::Reta(), 0.f, 1.8f, Mira(Mira::MirarNoJogador));
                // TESTE - LACO: uma forma escrita, 5 lacos girados (um por
                // projetil do leque), pelo alinhamento automatico do caminho.
                p->insertMotion<PathBehavior>(PathShapes::Loop(90.f, 400.f, 6), 600.f);
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
    paramsFast->numProjectiles = 3;
    paramsFast->projectileSpeed = 340.0f;
    paramsFast->angle = 40.f;

    auto spawner = boss->GetProjectileFactory("Capivara");

    boss->AddAttackPattern(STATE_NAME,
        std::make_unique<AngledAttack>(spawner, boss),
        std::move(paramsFast),
        0.8f,
        [](Projectile* p, int i) {
            {   // ORIGINAL: if (rand<0.1) insertMotion<PathBehavior>(PathShapes::Reta(), 0.f, 1.2f, Mira(Mira::MirarNoJogador));
                // TESTE - ARCO largo.
                const float lat = (i % 2 == 0) ? 135.f : -135.f;
                p->insertMotion<PathBehavior>(PathShapes::Arc(540.f, lat, 10), 195.f);
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
            {   // ORIGINAL: if (rand<0.4) insertMotion<PathBehavior>(PathShapes::Reta(), 0.f, 1.6f, Mira(Mira::MirarNoJogador));
                // TESTE - AS TRES FORMAS no mesmo disparo, uma por projetil.
                switch (i % 3) {
                    case 0:  p->insertMotion<PathBehavior>(PathShapes::Loop(80.f, 500.f, 12), 175.f); break;
                    case 1:  p->insertMotion<PathBehavior>(PathShapes::Arc(480.f, 115.f, 10), 190.f); break;
                    default: p->insertMotion<PathBehavior>(PathShapes::Zigzag(80.f, 45.f, 6), 200.f); break;
                }
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


