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

Boss::Boss(Scene* scene) : Actor(scene) {

    AddComponent<RigidBodyComponent>();
    AddComponent<FSMComponent>();

}

Boss::~Boss() = default;

void Boss::Start() {
    if (auto fsm = GetComponent<FSMComponent>()) {
        if (auto battleScene = dynamic_cast<Battle*>(mScene)) {
            fsm->SetOnStateChanged([battleScene](float newDuration) {
                battleScene->ResetHUDTimer(newDuration);
            });
        }

        if (!mInitialState.empty()) {
            fsm->Start(mInitialState);
        }
        else {
            SDL_Log("Boss::Start(): Nenhum estado inicial foi definido pela Factory");
        }
    }
    else {
        SDL_Log("Erro: Boss::Start() nao encontrou um FSMComponent");
    }
}

void Boss::OnUpdate(float deltaTime) {
    Actor::OnUpdate(deltaTime);

    auto fsm = GetComponent<FSMComponent>();
    if (!fsm) return;

    std::string currState = fsm->GetStateName();
    if (currState.empty()) { return; }

    // Verifica se existem ataques registrados pro estado atual
    if (mAttacksMap.find(currState) != mAttacksMap.end()) {

        // Itera sobre todos os ataques configurados para esse estado
        for (auto& attackDefinition : mAttacksMap[currState]) {

            attackDefinition.currentTimer -= deltaTime;

            if (attackDefinition.currentTimer <= 0.0f) {
                ExecuteAttack(attackDefinition, currState);
                attackDefinition.currentTimer = attackDefinition.cooldownTotal;
            }
        }
    }
}

void Boss::AddAttackPattern(const std::string &stateName, std::unique_ptr<IAttackStrategy> strategy, AttackParams params, float cooldown, ProjectileConfigurator config) {
    AttackDefinition def;
    def.strategy = std::move(strategy);
    def.params = params;
    def.configurator = std::move(config);
    def.cooldownTotal = cooldown;
    def.currentTimer = 0.0f;

    mAttacksMap[stateName].push_back(std::move(def));
}


void Boss::ExecuteAttack(AttackDefinition& attackDef, const std::string& stateName)
{
    // Atualiza a posição de tiro pra posição do boss.
    attackDef.params.firePosition = GetPosition();

    // Chama customização do boss específico
    CustomizeAttackParams(attackDef.params, stateName);

    auto battleScene = dynamic_cast<Battle*>(mScene);
    if (!battleScene) { return; }

    auto projManager = battleScene->GetProjectileManager();
    if (!projManager) { return; }

    // Executar estratégia
    auto projectiles = attackDef.strategy->Execute(attackDef.params);

    // Depois de executar, aplicar a personalização lambda, se existir
    if (attackDef.configurator) {
        for (int i = 0; i < projectiles.size(); ++i) {
            attackDef.configurator(projectiles[i].get(), i);
        }
    }

    // Converte pra BossProjectile e envia para o Manager
    std::vector<std::unique_ptr<BossProjectile>> bossProjVector;
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

void Boss::SetInitialState(const std::string &stateName) {
    mInitialState = stateName;
}




