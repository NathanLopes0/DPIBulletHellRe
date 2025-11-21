
#include "SallesFactory.h"
#include "../../Teacher/Bosses/Salles.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Attacks/BaseStrategies/AngledAttack.h"
#include "../../../Attacks/BaseStrategies/CircleSpreadAttack.h"
#include "../../../Attacks/BaseStrategies/WaveAttack.h"
#include "../../../Actors/Teacher/BossFactory/BossProjectileFactory/SallesProjectile1Factory.h"
#include "../../../Attacks/Behaviors.h"
#include "../../Projectile.h"
#include "../../../Random.h"
#include "../BossAttackState.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/ColliderComponents/CircleColliderComponent.h"

#define STATE_ONE_DURATION 1.f
#define STATE_TWO_DURATION 1.f
#define STATE_THREE_DURATION 1.f
#define STATE_FINAL_DURATION 17.f

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

    boss->SetProjectileFactory(std::make_unique<SallesProjectile1Factory>());

}


// Em SallesFactory.cpp
void SallesFactory::ConfigureAttacksAndFSM(Boss* boss) {


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

void SallesFactory::ConfigureStateOne(Boss* boss, FSMComponent* fsm, ProjectileFactory* spawner)
{

    const std::string STATE_NAME = "StateOne";

    // 1. Configura os parâmetros fixos
    AttackParams params;
    params.numProjectiles = 2;
    params.projectileSpeed = 230.0f;
    params.angle = 20.f;
    boss->AddAttackTemplate(STATE_NAME, params);
    boss->AddAttackCooldown(STATE_NAME, 0.3f);

    // 2. Configura a Lógica Customizada
    const ProjectileConfigurator config = [](Projectile* p, int index) {
        if (const auto chance = Random::GetFloatRange(0.0f, 1.0f); chance < 0.2) {
            p->insertBehavior<HomingBehavior>(1.2f, 280.f);
            p->GetComponent<DrawAnimatedComponent>()->SetAnimation("Homing");
        }
    };

    // 3. Adiciona o "Passo de Ataque"
    auto attack = std::make_unique<AngledAttack>(spawner, boss);
    boss->AddAttackStep(STATE_NAME, std::move(attack), config);

    // 4. Cria o objeto que representa o estado que terá esse ataque
    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
                                                      /*duration*/ STATE_ONE_DURATION,
                                                      /*nextState*/ "StateTwo");
    fsm->RegisterState(std::move(stateObj));
}


void SallesFactory::ConfigureStateTwo(Boss* boss, FSMComponent* fsm, ProjectileFactory* spawner)
{

    const std::string STATE_NAME = "StateTwo";

    // 1. Configura os parâmetros para este estado
    AttackParams params;
    params.numProjectiles = 3;
    params.projectileSpeed = 230.0f;
    params.angle = 30.f;


    boss->AddAttackTemplate(STATE_NAME, params);
    boss->AddAttackCooldown(STATE_NAME, 0.4f);

    // 2. Lógica Customizada
    ProjectileConfigurator config = [](Projectile* p, int index) {
        if (auto chance = Random::GetFloatRange(0.0f, 1.0f); chance < 0.3) {
            p->insertBehavior<HomingBehavior>(1.8f, 0.0f);
            p->GetComponent<DrawAnimatedComponent>()->SetAnimation("Homing");
        }
    };

    // 3. Adiciona o Passo
    auto attack = std::make_unique<AngledAttack>(spawner, boss);
    boss->AddAttackStep(STATE_NAME, std::move(attack), config);

    // 4. Cria o objeto que representa o estado desse ataque
    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
                                                      /*duration*/ STATE_TWO_DURATION,
                                                      /*nextState*/ "StateThree");
    fsm->RegisterState(std::move(stateObj));
}

void SallesFactory::ConfigureStateThree(Boss *boss, FSMComponent *fsm, ProjectileFactory *spawner) {
    const std::string STATE_NAME = "StateThree";

    // 1 . Configura os padrões fixos do estado três
    AttackParams params;
    params.numProjectiles = 4;
    params.projectileSpeed = 260.0f;
    params.angle = 40.f;

    boss->AddAttackTemplate(STATE_NAME, params);
    boss->AddAttackCooldown(STATE_NAME, 0.6f);

    ProjectileConfigurator config = [](Projectile* p, int index) {
        if (const auto chance = Random::GetFloatRange(0.0f, 1.0f); chance < 0.5) {
            p->insertBehavior<HomingBehavior>(1.8f, 0.0f);
            p->GetComponent<DrawAnimatedComponent>()->SetAnimation("Homing");
        }
    };

    auto attack = std::make_unique<AngledAttack>(spawner, boss);
    boss->AddAttackStep(STATE_NAME, std::move(attack), config);

    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
                                                  /*duration*/ STATE_THREE_DURATION,
                                                  /*nextState*/ "");

    fsm->RegisterState(std::move(stateObj));
}

void SallesFactory::ConfigureStateFinal(Boss *boss, FSMComponent *fsm, ProjectileFactory *spawner) {
    const std::string STATE_NAME = "StateFinal";

    // 1. Padrões fixos do estado Final

}


