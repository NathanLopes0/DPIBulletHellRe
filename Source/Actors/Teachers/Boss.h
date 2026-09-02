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

#include "../ProjectileFactory.h"
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
        std::unique_ptr<AttackParams> params;                       // Parâmetros (Speed, Angle, NumProj)
        ProjectileConfigurator configurator;       // Lambda para customização (Homing, Anim)
        float cooldownTotal;                       // Tempo entre disparos desse ataque
        float currentTimer;                        // Timer atual desse ataque específico
    };

    void AddAttackPattern(const std::string& stateName,
                          std::unique_ptr<IAttackStrategy> strategy,
                          std::unique_ptr<AttackParams> params,
                          float cooldown,
                          ProjectileConfigurator config = nullptr);

    void SetInitialState(const std::string& stateName);

    [[nodiscard]] ProjectileFactory* GetProjectileFactory(const std::string& factoryName) const;

    void AddProjectileFactory(const std::string& projectileName, std::unique_ptr<ProjectileFactory> factory);

    /**
     * @brief Retorna os nomes de TODAS as ProjectileFactory registradas por
     * este Boss (ex: {"Baloes", "Arduino", ...}). Usado por sistemas
     * externos como a tela de carregamento (Battle::UpdateLoadingStep) para
     * saber quantos/quais tipos existem sem hardcoding — cada Boss concreto
     * pode registrar tipos diferentes, e esta lista reflete exatamente
     * mProjectileFactories no momento da chamada.
     */
    [[nodiscard]] std::vector<std::string> GetProjectileFactoryNames() const;

    /**
     * @brief Pré-aquece TODAS as ProjectileFactory registradas por este Boss,
     * criando 'countPerType' instâncias de cada tipo e devolvendo-as aos
     * respectivos pools. Pensado para ser chamado numa tela de carregamento,
     * ANTES da batalha começar — depois disso, os primeiros disparos de cada
     * tipo já encontram objetos prontos no pool, sem custo de alocação nem
     * de carregar textura/JSON durante o gameplay real.
     * @param countPerType Quantas instâncias criar para CADA tipo de projétil
     * registrado (não é um total agregado — é por tipo).
     */
    void PrewarmProjectilePools(int countPerType);

    [[nodiscard]] Vector2 GetDirectionToPlayer() const;

    void RegisterMovementStrategy(const std::string& stateName, std::unique_ptr<IMovementStrategy> strategy);


protected:
    virtual void CustomizeAttackParams(AttackParams& params, const std::string& stateName);


private:
    // Função interna para executar um ataque específico
    virtual void ExecuteAttack(AttackDefinition& attackDef, const std::string& stateName);

    void CalculateNextDropThreshold();
    int mHitCounter = 0;
    int mNextDropThreshold = 0;

    std::string mInitialState;

    // Mapeia: Nome do projétil -> Fábrica dele (Populado na Fábrica)
    std::unordered_map<std::string, std::unique_ptr<ProjectileFactory>> mProjectileFactories;

    // Mapeia: Nome do Estado -> Lista de Definições de Ataque
    std::map<std::string, std::vector<AttackDefinition>> mAttacksMap;

    std::map<std::string, std::unique_ptr<IMovementStrategy>> mMovementStrategies;
    IMovementStrategy* mCurrentMovementStrategy = nullptr;
    std::string mLastStateName {}; // Para detectar mudança de estado

};