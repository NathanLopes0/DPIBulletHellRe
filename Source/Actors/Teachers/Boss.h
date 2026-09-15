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

    /**
     * @brief Direcao ate onde o jogador ESTARA daqui a leadTime segundos,
     * extrapolando pela velocidade atual dele.
     *
     * Com leadTime = 0 e identico a GetDirectionToPlayer(). Quanto maior, mais
     * o boss "chuta na frente" do jogador.
     *
     * O efeito de jogo e o que interessa: quem anda em linha reta (ou fica
     * parado) e acertado, e quem troca de direcao no momento do disparo faz o
     * tiro passar longe. Isso ensina o jogador a variar o movimento sem
     * precisar explicar nada a ele.
     *
     * @param leadTime segundos de antecipacao. O jogador anda a 300 px/s,
     *        entao 0.5f equivale a prever cerca de 150 px a frente: desvio bem
     *        visivel na tela, mas ainda desviavel.
     */
    [[nodiscard]] Vector2 GetPredictedPlayerDirection(float leadTime) const;

    /**
     * @brief Distancia em pixels ate o jogador. Devolve 0 se nao houver
     * jogador. Util para calcular tempo de voo de projetil.
     */
    [[nodiscard]] float GetDistanceToPlayer() const;

    void RegisterMovementStrategy(const std::string& stateName, std::unique_ptr<IMovementStrategy> strategy);


protected:
    virtual void CustomizeAttackParams(AttackParams& params, const std::string& stateName);

    /**
     * @brief Gancho chamado logo DEPOIS de a strategy criar os projeteis e
     * ANTES de eles serem entregues ao ProjectileManager.
     *
     * Este e o ponto de extensao para customizacoes que valem para a leva
     * inteira e que a strategy nao tem como saber - o Andre, por exemplo,
     * sorteia UMA cor por rajada.
     *
     * Existe porque a alternativa era sobrescrever ExecuteAttack por completo,
     * que foi o que o Andre fazia: 59 linhas copiadas da base para inserir 8.
     * O custo disso nao e estetico - a copia ja tinha ficado para tras e
     * perdido a guarda de params/strategy nulos que a base ganhou depois.
     *
     * @param projectiles Os projeteis recem-criados, ainda como Projectile.
     * @param stateName Nome do estado da FSM que disparou o ataque.
     */
    virtual void OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                      const std::string& stateName);

private:
    // Função interna para executar um ataque específico.
    // NAO e virtual de proposito: a sequencia de passos (customizar params ->
    // executar strategy -> configurator -> converter -> entregar ao manager ->
    // audio) e a mesma para todo boss. Quem precisa variar usa os ganchos
    // CustomizeAttackParams e OnProjectilesCreated.
    void ExecuteAttack(AttackDefinition& attackDef, const std::string& stateName);

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