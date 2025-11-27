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

RicardoFactory::RicardoFactory(Game *game)
    : IBossFactory(game)
{
}

std::unique_ptr<Boss> RicardoFactory::InstantiateBoss(Scene *scene) {
    SDL_Log("Instantiating Boss");
    return std::make_unique<Ricardo>(scene);
}

void RicardoFactory::ConfigureComponents(Boss *boss) {

    // ----- DRAW COMPONENT ----- //
    SDL_Log("RICARDO FACTORY - CONFIGURANDO DRAWCOMPONENT");
    auto drawComp = boss->AddComponent<DrawAnimatedComponent>("../Assets/Teachers/DPIBHRicardo.png",
        "../Assets/Teachers/DPIBHRicardo.json");
    drawComp->AddAnimation("Idle", {0});
    drawComp->SetAnimation("Idle");

    SDL_Log("Draw Configurado, configurando Colliders");
    // ----- COLLIDER COMPONENT ----- //
    const float colliderRadius = static_cast<float>(drawComp->GetSpriteWidth()) / 2.2f;
    const auto collider = boss->AddComponent<CircleColliderComponent>(colliderRadius);
    collider->SetTag(ColliderTag::Boss);

    SDL_Log("Collider Configurado, setando ProjectileFactory");
    boss->SetProjectileFactory(std::make_unique<RicardoProjectile1Factory>());
}

void RicardoFactory::ConfigureAttacksAndFSM(Boss *boss) {
    auto fsm = boss->GetComponent<FSMComponent>();
    if (!fsm) {
        SDL_Log("ERRO CRÍTICO: Boss não tem FSMComponent");
        return;
    }

    auto spawner = boss->GetProjectileFactory();
    if (!spawner) {
        SDL_Log("ERRO CRÍTICO: Boss não tem ProjectileFactory");
        return;
    }

    ConfigureStateOne(boss, fsm, spawner);
    ConfigureStateTwo(boss, fsm, spawner);
    ConfigureStateThree(boss, fsm, spawner);
    ConfigureStateFinal(boss, fsm, spawner);

    boss->SetInitialState("StateOne");
}

void RicardoFactory::ConfigureStateOne(Boss *boss, FSMComponent *fsm, ProjectileFactory *spawner) {

    const std::string STATE_NAME = "StateOne";

    // 1. Configurar parâmetros fixos
    AttackParams params;
    params.numProjectiles = 36;
    params.projectileSpeed = 180.f;

    boss->AddAttackPattern(STATE_NAME,
        std::make_unique<CircleSpreadAttack>(spawner, boss),
        params,
        2.4f
        );

    // Criar objeto que representa o estado que terá esse ataque
    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
                                                        STATE_ONE_DURATION,
                                                        "StateTwo");
    fsm->RegisterState(std::move(stateObj));

    boss->RegisterMovementStrategy(STATE_NAME, std::make_unique<RandomWanderStrategy>(4.f, 100.f));

}

void RicardoFactory::ConfigureStateTwo(Boss *boss, FSMComponent *fsm, ProjectileFactory *spawner) {
    const std::string STATE_NAME = "StateTwo";

    // 1. Configurar parâmetros fixos
    AttackParams params;
    params.numProjectiles = 36;
    params.projectileSpeed = 180.f;

    boss->AddAttackPattern(STATE_NAME,
        std::make_unique<CircleSpreadAttack>(spawner, boss),
        params,
        2.4f,
        [](Projectile* p, int index) {
            p->insertBehavior<SlowDownBehavior>(1.3f, 0.8f);
            p->insertBehavior<HomingBehavior>(1.5f, 180.f);
        }
        );

    // Criar objeto que representa o estado que terá esse ataque
    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
                                                        STATE_TWO_DURATION,
                                                        "StateThree");
    fsm->RegisterState(std::move(stateObj));

    boss->RegisterMovementStrategy(STATE_NAME, std::make_unique<RandomWanderStrategy>(3.f, 160.f));
}

void RicardoFactory::ConfigureStateThree(Boss *boss, FSMComponent *fsm, ProjectileFactory *spawner) {

    const std::string STATE_NAME = "StateThree";

    AttackParams params;
    params.numProjectiles = 36;
    params.projectileSpeed = 200.f;

    boss->AddAttackPattern(STATE_NAME,
    std::make_unique<CircleSpreadAttack>(spawner, boss),
    params,
    2.f,
    [](Projectile* p, const int index) {
        if (index % 2 == 0)
            p->insertBehavior<HomingBehavior>(1.f, 200.f);
    }
    );

    // Criar objeto que representa o estado que terá esse ataque
    auto stateObj = std::make_unique<BossAttackState>(fsm, STATE_NAME,
                                                        STATE_THREE_DURATION,
                                                        "");
    fsm->RegisterState(std::move(stateObj));

    boss->RegisterMovementStrategy(STATE_NAME, std::make_unique<RandomWanderStrategy>(2.f, 180.f));
}

void RicardoFactory::ConfigureStateFinal(Boss *boss, FSMComponent *fsm, ProjectileFactory *spawner) {

}


