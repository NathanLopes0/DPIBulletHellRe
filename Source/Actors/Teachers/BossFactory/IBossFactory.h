//
// Created by gensh on 17/10/2025.
//

#pragma once

#include <memory>
#include <string>

class Boss;
class Scene;
class Game;

/**
 * @interface IBossFactory
 * @brief A receita de montar um chefe (Template Method): CreateBoss chama os
 * passos na ordem certa, e quem varia sao os passos.
 *
 * A RECEITA E A MESMA PARA OS ONZE: o desenho e o colisor a partir do
 * retrato, os projeteis de Assets/Attacks/<pasta>/projeteis.json e as fases
 * de fases.json. Por isso os dois passos de montagem tem corpo AQUI, e o que
 * cada chefe diz sao dados (Pasta, Retrato) e a propria classe
 * (InstantiateBoss) - ver FabricaDeChefe.h.
 *
 * Os passos continuam virtuais: um chefe que um dia precise de montagem
 * propria herda daqui e sobrescreve so o passo que muda.
 */
class IBossFactory {

public:
    explicit IBossFactory(Game* game);
    virtual ~IBossFactory() = default;

    /**
     * @brief O metodo principal da fabrica. Executa a "receita" de criacao.
     * Esta funcao NAO e virtual: e o "Template" que chama os passos virtuais
     * na ordem correta.
     * @param scene A cena onde o Boss sera criado.
     * @return O Boss montado e pronto, ou nulo se a classe nao pode ser criada.
     */
    std::unique_ptr<Boss> CreateBoss(Scene* scene);

    /**
     * @brief O cartao do professor, sem a extensao: "Teachers/DPIBH<Nome>", com
     * o .png e o .json do DrawAnimatedComponent ao lado. Quem so quer mostrar o
     * rosto - a escolha do orientador, no TCC - le daqui sem montar o chefe.
     */
    [[nodiscard]] virtual std::string Retrato() const = 0;

protected:

    /// @brief A pasta do chefe em Assets/Attacks/ ("salles", "hugo"...): de la
    /// vem os projeteis, as regras e as fases.
    [[nodiscard]] virtual const std::string& Pasta() const = 0;

    /** @brief PASSO 1: Criar e retornar o Boss puro */
    virtual std::unique_ptr<Boss> InstantiateBoss(Scene* scene) = 0;

    /** @brief PASSO 2: o desenho, o colisor e os projeteis. */
    virtual void ConfigureComponents(Boss* boss);

    /** @brief PASSO 3: as fases (a FSM) e os ataques de cada uma. */
    virtual void ConfigureAttacksAndFSM(Boss* boss);

    Game* mGame;
};
