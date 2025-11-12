//
// Created by nslop on 25/09/2024.
//

#include "Boss.h"
#include "../../Scenes/Battle/Battle.h"
#include "../../Actors/Projectile.h"
#include "../ProjectileFactory.h"
#include "../../Scenes/Battle/ProjectileManager.h"
#include "../Player/Player.h"
#include "../../Actors/Teacher/Bosses/BossesProjectiles/BossProjectile.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Components/AIComponents/FSMComponent.h"
#include "SDL_log.h"

Boss::Boss(Scene* scene) : Actor(scene), mAttackCooldown(0.0f) {

    AddComponent<RigidBodyComponent>();
    AddComponent<FSMComponent>();

}

Boss::~Boss() = default;

void Boss::Start() {
    SDL_Log("Starting Boss dentro de Boss.cpp");
}

void Boss::OnUpdate(float deltaTime) {
    Actor::OnUpdate(deltaTime);

    mAttackCooldown -= deltaTime;
    if (mAttackCooldown <= 0.0f) {
        Attack();
    }
}


void Boss::AddAttackStep(const std::string &stateName,
                        std::unique_ptr<IAttackStrategy> strategy,
                        ProjectileConfigurator configurator) {

    AttackStep newStep;
    newStep.strategy = std::move(strategy);
    newStep.configurator = std::move(configurator);

    mAttackSteps[stateName].emplace_back(std::move(newStep));

}


void Boss::AddAttackTemplate(const std::string &stateName, const AttackParams &paramsTemplate) {
    mAttackParamTemplates[stateName] = paramsTemplate;
}

void Boss::AddAttackCooldown(const std::string &stateName, float cooldown) {
    mStateCooldowns[stateName] = cooldown;
}


void Boss::Attack()
{

   auto fsm = GetComponent<FSMComponent>();
    if (!fsm) {
        SDL_Log("Erro em Boss.cpp - Attack(): Nao foi possivel resgatar o FSMComponent do Boss");
        return;
    }

    auto stateName = fsm->GetStateName();
    if (stateName.empty()) {
        return;
    }

    AttackParams params = mAttackParamTemplates.at(stateName);
    params.firePosition = GetPosition();

    CustomizeAttackParams(params, stateName); //Momento da classe filha chamar seu Customize personalizado e mudar

    auto& attackSteps = mAttackSteps.at(stateName);
    std::vector<std::unique_ptr<BossProjectile>> bossProjVector;

    auto battleScene = dynamic_cast<Battle*>(mScene);
    auto projManager = battleScene->GetProjectileManager();
    if (!projManager) { SDL_Log("ProjectileManager nao encontrado"); return; }

    for (const auto& step : attackSteps) {
        auto projectiles = step.strategy->Execute(params);
        if (step.configurator) {
            for (int i = 0; i < projectiles.size(); ++i) {
                step.configurator(projectiles[i].get(), i);
            }
        }

        bossProjVector.reserve(projectiles.size());

        for (auto& p : projectiles) {
            if (dynamic_cast<BossProjectile*>(p.get())) {
                bossProjVector.emplace_back(std::unique_ptr<BossProjectile>(dynamic_cast<BossProjectile*>(p.release())));
            }
        }

        if (!bossProjVector.empty()) {
            projManager->AddBossProjectiles(std::move(bossProjVector));
        }
    }
    ResetAttackCooldown(stateName);
}

void Boss::ResetAttackCooldown(const std::string &stateName) {
    if (mStateCooldowns.count(stateName)) {
        mAttackCooldown = mStateCooldowns.at(stateName);
    } else {
        SDL_Log("Erro em Boss.cpp - ResetAttackCooldown: Cooldown nao encontrado para o estado '%s'", stateName.c_str());
        mAttackCooldown = 2.0f; //Padrão de Segurança
    }
}

Vector2 Boss::GetDirectionToPlayer() {
    if (auto battleScene = dynamic_cast<Battle*>(mScene)) {
        if (auto player = battleScene->GetPlayer()) {
            Vector2 direction = player->GetPosition() - GetPosition();
            direction.Normalize();
            return direction;
        }
    }

    //Failsafe: se não achar o player, atira pra baixo.
    return Vector2(0.0f, 1.0f);
}

void Boss::SetProjectileFactory(std::unique_ptr<ProjectileFactory> factory) {
    mProjectileFactory = std::move(factory); // Transfere a posse
}

ProjectileFactory* Boss::GetProjectileFactory() {
    if (mProjectileFactory) {
        return mProjectileFactory.get(); // Retorna o ponteiro cru (observador)
    }
    return nullptr;
}

void Boss::CustomizeAttackParams(AttackParams &params, const std::string &stateName) {

}



