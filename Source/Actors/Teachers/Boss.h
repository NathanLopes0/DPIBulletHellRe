//
// Created by nslop on 25/09/2024.
//

#pragma once

#include <functional>

#include "../Actor.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include "../../Attacks/IAttackStrategy.h"
#include "../../Attacks/AttackParameters/AttackParams.h"
#include "Bosses/BossesProjectiles/BossProjectile.h"

class IMovementStrategy;
class FSMComponent;
class ProjectileManager;
class Player;
class ProjectileFactory;
class Projectile;

using ProjectileConfigurator = std::function<void(Projectile* projectile, int index)>;


class Boss : public Actor {
public:

    explicit Boss(Scene* scene);
    ~Boss() override;


    // --- Ciclo de Vida ---
    void Start();
    void OnUpdate(float deltaTime) override;
    void OnCollision(Actor *other) override;

    struct AttackDefinition {
        std::unique_ptr<IAttackStrategy> strategy; // A lógica (Angled, Circle, etc)
        AttackParams params;                       // Parâmetros (Speed, Angle, NumProj)
        ProjectileConfigurator configurator;       // Lambda para customização (Homing, Anim)
        float cooldownTotal;                       // Tempo entre disparos desse ataque
        float currentTimer;                        // Timer atual desse ataque específico
    };

    void AddAttackPattern(const std::string& stateName,
                          std::unique_ptr<IAttackStrategy> strategy,
                          AttackParams params,
                          float cooldown,
                          ProjectileConfigurator config = nullptr);

    void SetInitialState(const std::string& stateName);
    void SetProjectileFactory(std::unique_ptr<ProjectileFactory> factory);
    ProjectileFactory* GetProjectileFactory() const;

    Vector2 GetDirectionToPlayer() const;

    void RegisterMovementStrategy(const std::string& stateName, std::unique_ptr<IMovementStrategy> strategy);


protected:
    virtual void CustomizeAttackParams(AttackParams& params, const std::string& stateName);


private:
    // Função interna para executar um ataque específico
    void ExecuteAttack(AttackDefinition& attackDef, const std::string& stateName);

    void CalculateNextDropThreshold();
    int mHitCounter = 0;
    int mNextDropThreshold = 0;

    std::string mInitialState;
    std::unique_ptr<ProjectileFactory> mProjectileFactory;

    // Mapeia: Nome do Estado -> Lista de Definições de Ataque
    std::map<std::string, std::vector<AttackDefinition>> mAttacksMap;

    std::map<std::string, std::unique_ptr<IMovementStrategy>> mMovementStrategies;
    IMovementStrategy* mCurrentMovementStrategy = nullptr;
    std::string mLastStateName {}; // Para detectar mudança de estado

};