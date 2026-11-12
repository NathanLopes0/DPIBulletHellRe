//
// A condicao de um experimento: as mecanicas que esta execucao desliga.
//

#include "doctest.h"

#include <string>
#include <vector>

#include "../Source/Pesquisa/Experimento.h"

using namespace Experimento;

TEST_CASE("Experimento: sem condicao, tudo ligado, e o nome e normal") {
    for (const char* texto : {"", "normal", "  normal "}) {
        CAPTURE(texto);
        std::vector<std::string> problemas;
        const Condicao c = Ler(texto, &problemas);
        CHECK(problemas.empty());
        for (const Mecanica m : kTodas) CHECK(c.Ligada(m));
        CHECK(c.Nome() == "normal");
    }
}

TEST_CASE("Experimento: cada nome desliga a sua, e o nome sai em ordem fixa") {
    const Condicao c = Ler("sem-papeis,sem-brilho");
    CHECK_FALSE(c.Ligada(Mecanica::Brilho));
    CHECK_FALSE(c.Ligada(Mecanica::Papeis));
    CHECK(c.Ligada(Mecanica::Cobertura));
    CHECK(c.Ligada(Mecanica::Raspao));
    // A ordem de kTodas, e nao a da linha de comando: a mesma condicao, o mesmo texto no CSV.
    CHECK(c.Nome() == "sem-brilho+sem-papeis");
    // O nome que sai tambem se le de volta.
    CHECK(Ler(c.Nome()).Nome() == c.Nome());
    // Todas as mecanicas tem nome, e o nome as desliga.
    for (const Mecanica m : kTodas) {
        CAPTURE(Nome(m));
        CHECK_FALSE(Ler(std::string("sem-") + Nome(m)).Ligada(m));
    }
}

TEST_CASE("Experimento: sem a cobertura, o brilho apaga junto - ele prometeria o que nao paga") {
    const Condicao c = Ler("sem-cobertura");
    CHECK_FALSE(c.Ligada(Mecanica::Cobertura));
    CHECK_FALSE(c.Ligada(Mecanica::Brilho));
    // O nome diz o que foi pedido.
    CHECK(c.Nome() == "sem-cobertura");
}

TEST_CASE("Experimento: o que nao se reconhece e relatado e ignorado") {
    std::vector<std::string> problemas;
    const Condicao c = Ler("sem-brilho, sem-musica,brilho", &problemas);
    CHECK_FALSE(c.Ligada(Mecanica::Brilho));
    REQUIRE(problemas.size() == 2u);
    CHECK(problemas[0].find("sem-musica") != std::string::npos);
    CHECK(problemas[1].find("brilho") != std::string::npos);
    CHECK(c.Nome() == "sem-brilho");
}
