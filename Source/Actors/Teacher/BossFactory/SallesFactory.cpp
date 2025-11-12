
#include "SallesFactory.h"
#include "../../Teacher/Bosses/Salles.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Attacks/BaseStrategies/AngledAttack.h"
#include "../../../Attacks/BaseStrategies/CircleSpreadAttack.h"
#include "../../../Actors/Teacher/BossFactory/BossProjectileFactory/SallesProjectile1Factory.h"
#include "../../../Attacks/Behaviors.h"
#include "../../Projectile.h"
#include "../../../Random.h"
#include "../BossAttackState.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/ColliderComponents/CircleColliderComponent.h"

#define STATE_ONE_DURATION 5.f
#define STATE_TWO_DURATION 17.f

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
    SDL_Log("%f", colliderRadius);
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

    boss->SetInitialState("StateOne");
}

void SallesFactory::ConfigureStateOne(Boss* boss, FSMComponent* fsm, ProjectileFactory* spawner)
{

    const std::string STATE_NAME = "StateOne";

    // 1. Configura os parâmetros fixos
    AttackParams params;
    params.numProjectiles = 15;
    params.projectileSpeed = 200.0f;
    boss->AddAttackTemplate(STATE_NAME, params);
    boss->AddAttackCooldown(STATE_NAME, 2.0f);

    // 2. Configura a Lógica Customizada
    const ProjectileConfigurator config = [](Projectile* p, int index) {
        if (const float chance = Random::GetFloatRange(0.0f, 1.0f); chance < 0.4f) { // 40% de chance
            p->insertBehavior<HomingBehavior>(0.9f, 0.0f);
            p->GetComponent<DrawAnimatedComponent>()->SetAnimation("Homing");
        }
    };

    // 3. Adiciona o "Passo de Ataque"
    auto attack = std::make_unique<CircleSpreadAttack>(spawner, boss);
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
    params.projectileSpeed = 200.0f;
    boss->AddAttackTemplate(STATE_NAME, params);
    boss->AddAttackCooldown(STATE_NAME, 2.0f);

    // 2. Lógica Customizada
    ProjectileConfigurator config = [](Projectile* p, int index) {
        float chance = Random::GetFloatRange(0.0f, 1.0f);
        if (chance < 0.5f) {
            p->GetComponent<DrawAnimatedComponent>()->SetAnimation("Homing");
            p->insertBehavior<SlowDownBehavior>(0.3f, 0.4f);
            p->insertBehavior<HomingBehavior>(0.6f, 300.f);
        }
    };

    // 3. Adiciona o Passo
    auto attack = std::make_unique<AngledAttack>(spawner, boss);
    boss->AddAttackStep(STATE_NAME, std::move(attack), config);

    // 4. Cria o objeto que representa o estado desse ataque
    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
                                                      /*duration*/ STATE_TWO_DURATION,
                                                      /*nextState*/ "StateOne"); // Volta pro 1
    fsm->RegisterState(std::move(stateObj));
}

