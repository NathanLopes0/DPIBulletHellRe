//
// Created by gensh on 17/10/2025.
//

#pragma once
#include <memory>
#include <vector>
#include "AttackParameters/AttackParams.h"
#include "../Actors/Actor.h" // Necessário (não apenas forward decl) para chamar mOwner->GetScene() em Acquire()
#include "../Actors/ProjectileFactory.h" // Necessário (não apenas forward decl) para chamar mSpawner->Acquire(...)

class Projectile;

/**
 * @interface IAttackStrategy
 * @brief Define o contrato para qualquer padrão de ataque no jogo.
 * Classes que herdam desta interface representam um padrão de ataque específico que um Boss pode executar.
 */
class IAttackStrategy {
public:

    // mSpawner e mOwner são comuns a TODA AttackStrategy concreta (antes eram
    // duplicados como membros privados idênticos em cada subclasse). Ficam
    // aqui para que Acquire() possa ser implementado uma única vez na base.
    IAttackStrategy(ProjectileFactory* spawner, Actor* owner)
        : mSpawner(spawner), mOwner(owner) {}

    virtual ~IAttackStrategy() = default;

    /**
    * @brief Executa o padrão de ataque.
     * @param params Um pacote de dados contendo todos os parâmetros
     * necessários para configurar o disparo (posição, velocidade, etc.).
     * @return Um vetor de ponteiros únicos para os projéteis recém-criados.
     */
    virtual std::vector<std::unique_ptr<Projectile>> Execute(const AttackParams& params) = 0;

protected:

    /**
     * @brief Pede um Projétil pronto (novo ou reciclado do pool) para o
     * mSpawner desta strategy, usando o mOwner já conhecido desde a
     * construção. É o método que toda AttackStrategy concreta deve chamar
     * dentro de Execute() no lugar do antigo mSpawner->createProjectile(scene, owner).
     * @return Um Projectile pronto (ActorState::Active), ou nullptr se o
     * spawner não conseguir montar um (ex: owner de tipo incompatível).
     */
    std::unique_ptr<Projectile> Acquire() const {
        return mSpawner->Acquire(mOwner->GetScene(), mOwner);
    }

    ProjectileFactory* mSpawner;
    Actor* mOwner;
};