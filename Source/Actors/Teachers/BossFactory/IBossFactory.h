//
// Created by gensh on 17/10/2025.
//

#pragma once

#include <memory>
#include <string>

class Boss;
class Scene;
class Game;

/// A receita de montar um chefe (Template Method): CreateBoss chama os passos na ordem certa, e
/// quem varia sao os passos.
class IBossFactory {

public:
    explicit IBossFactory(Game* game);
    virtual ~IBossFactory() = default;

    /// O metodo principal da fabrica. Executa a "receita" de criacao.
    std::unique_ptr<Boss> CreateBoss(Scene* scene);

    /// O cartao do professor, sem a extensao: "Teachers/DPIBH<Nome>", com o .png e o .json do
    /// DrawAnimatedComponent ao lado.
    [[nodiscard]] virtual std::string Retrato() const = 0;

protected:

    /// A pasta do chefe em Assets/Attacks/ ("salles", "hugo"...): de la vem os projeteis, as regras
    /// e as fases.
    [[nodiscard]] virtual const std::string& Pasta() const = 0;

    /// Passo 1: Criar e retornar o Boss puro.
    virtual std::unique_ptr<Boss> InstantiateBoss(Scene* scene) = 0;

    /// Passo 2: o desenho, o colisor e os projeteis.
    virtual void ConfigureComponents(Boss* boss);

    /// Passo 3: as fases (a FSM) e os ataques de cada uma.
    virtual void ConfigureAttacksAndFSM(Boss* boss);

    Game* mGame;
};
