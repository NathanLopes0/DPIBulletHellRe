//
// A fabrica de um chefe: a mesma receita para os onze, com a classe de cada um.
//

#pragma once

#include <memory>
#include <string>
#include <utility>

#include "IBossFactory.h"

/**
 * @class FabricaDeChefe
 * @brief A fabrica de qualquer chefe: so a classe, a pasta e o retrato mudam.
 *
 * ERAM ONZE CLASSES IGUAIS. Cada professor tinha a sua fabrica, com .h e .cpp,
 * e os onze ConfigureComponents e ConfigureAttacksAndFSM eram o mesmo codigo
 * letra por letra - medido: diferiam so no nome da pasta e nas mensagens de
 * log. Vinte e dois arquivos para um dado de tres campos; um chefe novo pedia
 * copiar mais dois, e uma correcao na receita teria de ser feita onze vezes.
 *
 * A RECEITA MORA EM IBossFactory, e o que e de cada chefe e DADO (a pasta e o
 * retrato) - registrado numa linha em Chefes.cpp. O que nao da para ser dado e
 * a classe do chefe, e e ela que o parametro do molde carrega.
 */
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
