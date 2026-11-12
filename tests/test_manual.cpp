//
// O manual do jogo: as paginas da tela "Como Jogar".
//

#include "doctest.h"

#include "../Source/Interface/Manual.h"

using namespace Manual;

TEST_CASE("Manual: le as paginas, com as linhas de teclado separadas") {
    const auto lido = LerManual(R"({
        // comentario vale, como nos outros arquivos de dados
        "paginas": [
            { "titulo": "Um", "linhas": ["a", "b"], "no_computador": ["Esc pausa"] },
            { "titulo": "Dois", "linhas": ["c"] }
        ]
    })");
    CHECK(lido.problemas.empty());
    REQUIRE(lido.paginas.size() == 2u);
    CHECK(lido.paginas[0].titulo == "Um");
    CHECK(lido.paginas[0].linhas == std::vector<std::string>{"a", "b"});
    CHECK(lido.paginas[0].noComputador == std::vector<std::string>{"Esc pausa"});
    CHECK(lido.paginas[1].noComputador.empty());
}

TEST_CASE("Manual: no gabinete, nenhuma linha fala de tecla") {
    Pagina p;
    p.titulo = "Controles";
    p.linhas = {"Botao 1: lancar Duvidas"};
    p.noComputador = {"Jogador 1: setas e Espaco"};
    CHECK(Linhas(p, true) == std::vector<std::string>{"Botao 1: lancar Duvidas"});
    CHECK(Linhas(p, false) == std::vector<std::string>{"Botao 1: lancar Duvidas", "Jogador 1: setas e Espaco"});
}

TEST_CASE("Manual: o que nao serve fica de fora, com o motivo, e o resto vale") {
    const auto lido = LerManual(R"({ "paginas": [
        { "titulo": "", "linhas": ["sem titulo"] },
        { "titulo": "Sem linhas", "linhas": [] },
        { "titulo": "Boa", "linhas": ["x", 3] },
        7
    ]})");
    REQUIRE(lido.paginas.size() == 1u);
    CHECK(lido.paginas[0].titulo == "Boa");
    CHECK(lido.paginas[0].linhas == std::vector<std::string>{"x"});   // o 3 nao e texto
    CHECK(lido.problemas.size() == 4u);

    CHECK(LerManual("nao e json").paginas.empty());
    CHECK_FALSE(LerManual("nao e json").problemas.empty());
    CHECK_FALSE(LerManual(R"({"outra": 1})").problemas.empty());
}

TEST_CASE("Manual: linhas demais sao cortadas na tela, com aviso na leitura") {
    std::string linhas;
    for (int i = 0; i < kLinhasNoMaximo + 2; ++i) linhas += std::string(i ? "," : "") + "\"l\"";
    const auto lido = LerManual(R"({"paginas":[{"titulo":"T","linhas":[)" + linhas + "]}]}");
    REQUIRE(lido.paginas.size() == 1u);
    CHECK(lido.problemas.size() == 1u);
    CHECK(static_cast<int>(Linhas(lido.paginas[0], false).size()) == kLinhasNoMaximo);
}

TEST_CASE("Manual: virar a pagina para nas pontas") {
    CHECK(Virar(0, 1, 6) == 1);
    CHECK(Virar(5, 1, 6) == 5);    // a ultima nao volta para a primeira
    CHECK(Virar(0, -1, 6) == 0);
    CHECK(Virar(3, -1, 6) == 2);
    CHECK(Virar(0, 1, 0) == 0);    // manual vazio
    CHECK(Indicador(2, 6) == "3 de 6");
}
