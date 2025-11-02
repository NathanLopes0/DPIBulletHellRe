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

    // --- Métodos de Configuração (usados pela Factory) ---
    void AddAttackTemplate(const std::string& stateName, const AttackParams& paramsTemplate);
    /** @brief Adiciona o valor de cooldown para um estado específico */
    void AddAttackCooldown(const std::string& stateName, float cooldown);
    /**
        * @brief Adiciona um "passo" de ataque (Estratégia + Lógica Customizada)
        * @param stateName O nome do estado (ex: "DefaultAttack", "StateOne")
        * @param strategy A Estratégia de Ataque base (ex: CircleSpreadAttack)
        * @param configurator A "Lógica Customizada" (ex: "fazer metade ter HomingBehavior")
    */
    void AddAttackStep(const std::string &stateName,
                       std::unique_ptr<IAttackStrategy> strategy,
                       ProjectileConfigurator configurator = nullptr); // O configurador é opcional

    // --- Ciclo de Vida ---
    void Start();
    void OnUpdate(float deltaTime) override;


    /**
     * @brief Define a fábrica de projéteis que este Boss usará.
     * A posse da fábrica é transferida para o Boss.
     */
    void SetProjectileFactory(std::unique_ptr<ProjectileFactory> factory);

    /**
     * @brief Obtém um ponteiro observador para a fábrica de projéteis do Boss.
     */
    ProjectileFactory* GetProjectileFactory();


protected:




    // --- Lógica de Combate ---
    virtual void Attack();
    void ResetAttackCooldown(const std::string& stateName);
    [[nodiscard]] virtual Vector2 GetDirectionToPlayer();

    struct AttackStep {
        std::unique_ptr<IAttackStrategy> strategy;
        ProjectileConfigurator configurator;
    };

    std::map<std::string, std::vector<AttackStep>> mAttackSteps;

    // O ARSENAL DE "KNOBS" (preenchido pela Factory)
    // Mapeia um NOME DE ESTADO para um TEMPLATE DE PARÂMETROS
    std::map<std::string, AttackParams> mAttackParamTemplates;

    // --- Lógica de Cooldown ---
    float mAttackCooldown;
    std::map<std::string, float> mStateCooldowns;

    std::unique_ptr<ProjectileFactory> mProjectileFactory;

};