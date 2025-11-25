//
// Created by gensh on 17/10/2025.
//

#pragma once

#include <memory>

#include "../../../Components/AIComponents/FSMComponent.h"

class ProjectileFactory;
class IAttackStrategy;
class Boss;
class Scene;
class Game;

/**
 * @interface IBossFactory
 * @brief Interface abstrata para todas as fábricas de Bosses.
 * Define um contrato para a criação de um Boss, garantindo o gerenciamento
 * de memória seguro através de std::unique_ptr.
 */

class IBossFactory {

public:
    explicit IBossFactory(Game* game);
    virtual ~IBossFactory() = default;

    /**
     * @brief O método principal da fábrica. Executa a "receita" de criação.
     * Esta função NÃO é virtual. Ela é o "Template" que chama
     * os "Passos" (helpers) virtuais na ordem correta.
     * As classes filhas vão mudar as funções internas
     * @param scene A cena onde o Boss será criado.
     * @return Um unique_ptr para o Boss 100% montado e pronto.
     */
    std::unique_ptr<Boss> CreateBoss(Scene* scene);

protected:

    // Os passos virtuais PUROS, todas as fábricas filhas
    // DEVEM implementar esses 3 passos.

    /** @brief PASSO 1: Criar e retornar o Boss puro */
    virtual std::unique_ptr<Boss> InstantiateBoss(Scene* scene) = 0;

    /** @brief PASSO 2: Instala todos os componentes necessários
     * para o boss (Draw, Collider, RigidBody, etc.) */
    virtual void ConfigureComponents(Boss* boss) = 0;

    /** @brief PASSO 3 : Instala a FSM e as AttackStrategies */
    virtual void ConfigureAttacksAndFSM(Boss* boss) = 0;

    virtual void ConfigureStateOne(Boss* boss, FSMComponent* fsm, ProjectileFactory* spawner) = 0;
    virtual void ConfigureStateTwo(Boss* boss, FSMComponent* fsm, ProjectileFactory* spawner) = 0;
    virtual void ConfigureStateThree(Boss* boss, FSMComponent* fsm, ProjectileFactory* spawner) = 0;
    virtual void ConfigureStateFinal(Boss* boss, FSMComponent* fsm, ProjectileFactory* spawner) = 0;

    Game* mGame;

};
