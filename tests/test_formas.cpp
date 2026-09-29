// Leitura de formas de trajetoria a partir de texto JSON.
// Camada pura: nao abre arquivo, nao escreve log, nao sobe SDL.

#include "doctest.h"
#include "../Source/Attacks/PathShapes.h"

using PathShapes::LerFormas;

TEST_CASE("Formas: le uma lista de pontos") {
    const auto r = LerFormas(R"({ "Serpente": { "pontos": [[60,-40],[120,-50],[160,-20]] } })");
    CHECK(r.problemas.empty());
    REQUIRE(r.formas.count("Serpente") == 1);
    REQUIRE(r.formas.at("Serpente").size() == 3);
    CHECK(r.formas.at("Serpente")[0].x == doctest::Approx(60.f));
    CHECK(r.formas.at("Serpente")[0].y == doctest::Approx(-40.f));
}

TEST_CASE("Formas: le um gerador, reaproveitando as formas em codigo") {
    const auto r = LerFormas(R"({ "Laco": { "gerador": {"tipo":"Loop","a":70,"b":500,"n":12} } })");
    CHECK(r.problemas.empty());
    REQUIRE(r.formas.count("Laco") == 1);
    // Loop gera 'n' pontos do circulo mais o ponto de fuga.
    CHECK(r.formas.at("Laco").size() == 13);
}

TEST_CASE("Formas: varias no mesmo arquivo") {
    const auto r = LerFormas(R"({
        "A": { "gerador": {"tipo":"Reta","a":800} },
        "B": { "pontos": [[10,0],[20,5]] }
    })");
    CHECK(r.problemas.empty());
    CHECK(r.formas.size() == 2);
}

// --- o arquivo nunca e descartado inteiro por causa de uma entrada errada ---

TEST_CASE("Formas: entrada invalida e ignorada, as boas continuam valendo") {
    const auto r = LerFormas(R"({
        "Boa":  { "pontos": [[10,0]] },
        "Ruim": { "gerador": {"tipo":"Espiral"} }
    })");
    CHECK(r.formas.count("Boa") == 1);
    CHECK(r.formas.count("Ruim") == 0);
    REQUIRE(r.problemas.size() == 1);
    // O problema diz QUAL forma e O QUE fazer - nao some em silencio.
    CHECK(r.problemas[0].find("Ruim") != std::string::npos);
    CHECK(r.problemas[0].find("Espiral") != std::string::npos);
}

TEST_CASE("Formas: ponto malformado e relatado com o nome da forma") {
    const auto r = LerFormas(R"({ "X": { "pontos": [[10,0],[20]] } })");
    CHECK(r.formas.empty());
    REQUIRE(r.problemas.size() == 1);
    CHECK(r.problemas[0].find("\"X\"") != std::string::npos);
}

TEST_CASE("Formas: falta pontos e gerador") {
    const auto r = LerFormas(R"({ "Y": { "cor": "azul" } })");
    CHECK(r.formas.empty());
    REQUIRE(r.problemas.size() == 1);
    CHECK(r.problemas[0].find("pontos") != std::string::npos);
}

TEST_CASE("Formas: lista de pontos vazia e recusada") {
    const auto r = LerFormas(R"({ "Z": { "pontos": [] } })");
    CHECK(r.formas.empty());
    CHECK(r.problemas.size() == 1);
}

TEST_CASE("Formas: JSON invalido nao derruba nada") {
    const auto r = LerFormas("{ isto nao e json");
    CHECK(r.formas.empty());
    REQUIRE(r.problemas.size() == 1);
    CHECK(r.problemas[0].find("JSON") != std::string::npos);
}

TEST_CASE("Formas: arquivo vazio nao e erro, so nao traz formas") {
    const auto r = LerFormas("{}");
    CHECK(r.formas.empty());
    CHECK(r.problemas.empty());
}

// ---------------------------------------------------------------------------
// Comentarios
// ---------------------------------------------------------------------------

TEST_CASE("Formas: o arquivo aceita comentario") {
    // O JSON padrao nao tem comentario, e sem ele migrar codigo comentado para
    // dados jogaria a explicacao fora. Ver Source/JsonDeDados.h.
    const auto r = LerFormas(R"({
        // Doze pontos: mais que isso e o projetil gasta quadro "chegando" em
        // cada waypoint e a curva fica truncada.
        "Laco": { "gerador": {"tipo":"Loop","a":70,"b":500,"n":12} }   // lento de proposito
        /* o bloco tambem vale */
    })");
    CHECK(r.problemas.empty());
    CHECK(r.formas.count("Laco") == 1);
}
