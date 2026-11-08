//
// A fabrica de um chefe: a mesma receita para os onze, com a classe de cada um.
//

#pragma once

#include <memory>
#include <string>
#include <utility>

#include "IBossFactory.h"

/// A fabrica de qualquer chefe: so a classe, a pasta e o retrato mudam.
template <typename Chefe>
class FabricaDeChefe final : public IBossFactory {
public:
    FabricaDeChefe(Game* game, std::string pasta, std::string retrato)
        : IBossFactory(game), mPasta(std::move(pasta)), mRetrato(std::move(retrato)) {}

    [[nodiscard]] std::string Retrato() const override { return mRetrato; }

protected:
    [[nodiscard]] const std::string& Pasta() const override { return mPasta; }

    std::unique_ptr<Boss> InstantiateBoss(Scene* scene) override {
        return std::make_unique<Chefe>(scene);
    }

private:
    std::string mPasta;
    std::string mRetrato;
};
