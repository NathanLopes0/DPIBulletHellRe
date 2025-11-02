//
// Created by gensh on 17/10/2025.
//

#pragma once
#include <memory>
#include <vector>
#include "AttackParameters/AttackParams.h"

class Projectile;
/**
 * @interface IAttackStrategy
 * @brief Define o contrato para qualquer padrão de ataque no jogo.
 * Classes que herdam desta interface representam um padrão de ataque específico que um Boss pode executar.
 */
class IAttackStrategy {
public:

    virtual ~IAttackStrategy() = default;

    /**
    * @brief Executa o padrão de ataque.
     * @param params Um pacote de dados contendo todos os parâmetros
     * necessários para configurar o disparo (posição, velocidade, etc.).
     * @return Um vetor de ponteiros únicos para os projéteis recém-criados.
     */
    virtual std::vector<std::unique_ptr<Projectile>> Execute(const AttackParams& params) = 0;
};