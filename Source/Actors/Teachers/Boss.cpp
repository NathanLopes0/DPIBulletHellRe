//
// Created by nslop on 25/09/2024.
//

#include "Boss.h"
#include "../../Scenes/Battle/Battle.h"
#include "../../Actors/Projectile.h"
#include "../ProjectileFactory.h"
#include "../../Scenes/Battle/ProjectileManager.h"
#include "../Player/Player.h"
#include "../../Actors/Teachers/Bosses/BossesProjectiles/BossProjectile.h"
#include "../../Movements/MovementStrategies.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Components/AIComponents/FSMComponent.h"
#include "SDL_log.h"
#include "../Player/PlayerProjectile.h"

Boss::Boss(Scene* scene) : Actor(scene) {

    AddComponent<RigidBodyComponent>();
    AddComponent<FSMComponent>();

}

Boss::~Boss() = default;

void Boss::Start() {
    CalculateNextDropThreshold();
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

    // --- LÓGICA DE MOVIMENTO ---
    // Verifica se mudou de estado para trocar a estratégia
    if (currState != mLastStateName) {
        mLastStateName = currState;

        // Procura se existe estratégia para o novo estado
        if (mMovementStrategies.find(currState) != mMovementStrategies.end()) {
            mCurrentMovementStrategy = mMovementStrategies[currState].get();
            mCurrentMovementStrategy->Initialize(this); // Reset timers/alvos
        } else {
            // Se não tiver estratégia definida, talvez parar o boss?
            mCurrentMovementStrategy = nullptr;
            if(auto rb = GetComponent<RigidBodyComponent>()) rb->SetVelocity(Vector2::Zero);
        }
    }

    // Executa a estratégia atual
    if (mCurrentMovementStrategy) {
        mCurrentMovementStrategy->Update(this, deltaTime);
    }

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
    def.currentTimer = 1.f;

    mAttacksMap[stateName].push_back(std::move(def));
}
void Boss::RegisterMovementStrategy(const std::string& stateName, std::unique_ptr<IMovementStrategy> strategy) {
    mMovementStrategies[stateName] = std::move(strategy);
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

    // -- LIDANDO COM O AUDIO DO ATAQUE -- //
    std::string attackAudio;
    {
        if (auto choose = Random::GetIntRange(0,1)) attackAudio = "tan00.wav";
        else attackAudio = "tan01.wav";
    }
    auto handler = mScene->GetGame()->GetAudio()->PlaySound(attackAudio, false);
    mScene->GetGame()->GetAudio()->SetSoundVolume(handler, 10);
    // ----------------------------- //

}

Vector2 Boss::GetDirectionToPlayer() const {
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

ProjectileFactory* Boss::GetProjectileFactory() const {
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

void Boss::CalculateNextDropThreshold() {
    int baseHits = 80;
    int variance = 8;

    mNextDropThreshold = baseHits + Random::GetIntRange(-variance, variance);

}

void Boss::OnCollision(Actor *other) {
    if (!dynamic_cast<PlayerProjectile*>(other)) { return; }

    mHitCounter++;
    if (mHitCounter == mNextDropThreshold) {
        if (auto battle = dynamic_cast<Battle*>(GetScene())) {
            battle->SpawnExtraPoint(GetPosition());
        }

        mHitCounter = 0;

        CalculateNextDropThreshold();
    }
}






