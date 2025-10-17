//
// Created by gensh on 17/10/2025.
//

#pragma once

#include <memory>

class Boss;
class Game;

/**
 * @interface IBossFactory
 * @brief Interface abstrata para todas as fábricas de Bosses.
 * Define um contrato para a criação de um Boss, garantindo o gerenciamento
 * de memória seguro através de std::unique_ptr.
 */
class IBossFactory {
public:
    // 1. Adicionado destrutor virtual. Essencial para classes base polimórficas.
    virtual ~IBossFactory() = default;

    //    - Retorna std::unique_ptr<Boss> para transferir a posse de forma segura.
    //    - Recebe uma referência a Game para obter contexto, em vez de um ponteiro.
    //    - O parâmetro Scene* foi removido para desacoplar a fábrica da cena.
    virtual std::unique_ptr<Boss> Create(Game& gameContext) const = 0;
};