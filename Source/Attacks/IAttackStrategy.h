//
// Created by gensh on 17/10/2025.
//

#pragma once

/**
 * @interface IAttackStrategy
 * @brief Define o contrato para qualquer padrão de ataque no jogo.
 * Classes que herdam desta interface representam um padrão de ataque específico que um Boss pode executar.
 */
class IAttackStrategy {
public:

    virtual ~IAttackStrategy() = default;

    virtual void Execute() = 0;
};