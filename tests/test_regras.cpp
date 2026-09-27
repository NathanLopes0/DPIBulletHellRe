// Leitura de regras de ataque a partir de texto JSON, e a decisao de a quem
// cada regra se aplica. Camada pura: sem arquivo, sem log, sem SDL.

#include "doctest.h"
#include "../Source/Attacks/RegrasDeAtaque.h"

TEST_CASE("Regras: le uma regra por indice com uma Motion") {
    const auto r = LerRegras(R"({
        "julio_fase2": [
            { "indices": [0,2], "motion": {"tipo":"Tracking","atraso":0.3,"forca":3.2,"duracao":2.4} }
        ]
    })");
    CHECK(r.problemas.empty());
    REQUIRE(r.conjuntos.count("julio_fase2") == 1);
    REQUIRE(r.conjuntos.at("julio_fase2").size() == 1);

    const Regra& regra = r.conjuntos.at("julio_fase2")[0];
    CHECK(regra.condicao == Regra::Indices);
    CHECK(regra.temMotion);
    CHECK(regra.motion.tipo == "Tracking");
    CHECK(regra.motion.forca == doctest::Approx(3.2f));
}

TEST_CASE("Regras: le chance, modifiers e animacao juntos") {
    const auto r = LerRegras(R"({
        "salles": [
            { "chance": 0.2,
              "modifiers": [ {"tipo":"SlowDown","atraso":1.3,"fator":0.8} ],
              "motion": {"tipo":"Path","forma":"Reta","velocidade":280,"atraso":1.2,"mira":"MirarNoJogador"},
              "animacao": "Homing" }
        ]
    })");
    CHECK(r.problemas.empty());
    const Regra& regra = r.conjuntos.at("salles")[0];
    CHECK(regra.condicao == Regra::Chance);
    CHECK(regra.chance == doctest::Approx(0.2f));
    REQUIRE(regra.modifiers.size() == 1);
    CHECK(regra.modifiers[0].tipo == "SlowDown");
    CHECK(regra.motion.mira == "MirarNoJogador");
    CHECK(regra.animacao == "Homing");
}

TEST_CASE("Regras: le o ritmo da investida") {
    const auto r = LerRegras(R"({
        "cacador": [
            { "motion":    {"tipo":"MiraPeriodica","ritmo":{"investida":0.2,"pausa":1.0,"repeticoes":6}},
              "modifiers": [{"tipo":"PulsoDeVelocidade","ritmo":{"investida":0.2,"pausa":1.0,"repeticoes":6},
                             "investida":640,"pausa":15}] }
        ]
    })");
    CHECK(r.problemas.empty());
    const Regra& regra = r.conjuntos.at("cacador")[0];
    CHECK(regra.motion.ritmo.repeticoes == 6);
    CHECK(regra.motion.ritmo.DuracaoTotal() == doctest::Approx(6 * 0.2f + 5 * 1.0f));
    CHECK(regra.modifiers[0].moduloInvestida == doctest::Approx(640.f));
}

// ---------------------------------------------------------------------------
// A divisao da fase 1, agora conferida na leitura
// ---------------------------------------------------------------------------

TEST_CASE("Regras: um Modifier em \"motion\" e recusado") {
    // Em C++ isso nem compila (static_assert da fase 1). Num arquivo de texto
    // nao ha compilador, entao a leitura precisa fazer o mesmo papel.
    const auto r = LerRegras(R"({ "x": [ { "motion": {"tipo":"SlowDown","fator":0.8} } ] })");
    CHECK(r.conjuntos.at("x").empty());
    REQUIRE(r.problemas.size() == 1);
    CHECK(r.problemas[0].find("SlowDown") != std::string::npos);
    CHECK(r.problemas[0].find("Modifier") != std::string::npos);
}

TEST_CASE("Regras: uma Motion em \"modifiers\" e recusada") {
    const auto r = LerRegras(R"({ "x": [ { "modifiers": [{"tipo":"Tracking"}] } ] })");
    CHECK(r.conjuntos.at("x").empty());
    REQUIRE(r.problemas.size() == 1);
    CHECK(r.problemas[0].find("Tracking") != std::string::npos);
}

TEST_CASE("Regras: tipo inexistente e recusado dizendo qual e") {
    const auto r = LerRegras(R"({ "x": [ { "motion": {"tipo":"Teletransporte"} } ] })");
    REQUIRE(r.problemas.size() == 1);
    CHECK(r.problemas[0].find("Teletransporte") != std::string::npos);
}

TEST_CASE("Regras: uma regra ruim nao derruba as boas do mesmo conjunto") {
    const auto r = LerRegras(R"({
        "x": [
            { "motion": {"tipo":"Tracking","forca":2} },
            { "motion": {"tipo":"NaoExiste"} },
            { "animacao": "Homing" }
        ]
    })");
    CHECK(r.conjuntos.at("x").size() == 2);
    CHECK(r.problemas.size() == 1);
}

TEST_CASE("Regras: uma regra que nao faz nada e recusada") {
    const auto r = LerRegras(R"({ "x": [ { "chance": 0.5 } ] })");
    CHECK(r.conjuntos.at("x").empty());
    REQUIRE(r.problemas.size() == 1);
    CHECK(r.problemas[0].find("nao faz nada") != std::string::npos);
}

TEST_CASE("Regras: JSON invalido nao derruba nada") {
    const auto r = LerRegras("{ isto nao e json");
    CHECK(r.conjuntos.empty());
    CHECK(r.problemas.size() == 1);
}

// ---------------------------------------------------------------------------
// A quem cada regra se aplica - sorteio injetado, entao testavel
// ---------------------------------------------------------------------------

TEST_CASE("RegraSeAplica: sem condicao, vale para todos") {
    Regra r;
    CHECK(RegraSeAplica(r, 0, 0.99f));
    CHECK(RegraSeAplica(r, 7, 0.01f));
}

TEST_CASE("RegraSeAplica: chance compara com o sorteio recebido") {
    Regra r; r.condicao = Regra::Chance; r.chance = 0.2f;
    CHECK(RegraSeAplica(r, 0, 0.19f));
    CHECK_FALSE(RegraSeAplica(r, 0, 0.21f));
}

TEST_CASE("RegraSeAplica: indices, pares e impares") {
    Regra i; i.condicao = Regra::Indices; i.indices = {0, 2};
    CHECK(RegraSeAplica(i, 2, 0.f));
    CHECK_FALSE(RegraSeAplica(i, 1, 0.f));

    Regra p; p.condicao = Regra::Pares;
    CHECK(RegraSeAplica(p, 4, 0.f));
    CHECK_FALSE(RegraSeAplica(p, 3, 0.f));

    Regra im; im.condicao = Regra::Impares;
    CHECK(RegraSeAplica(im, 3, 0.f));
    CHECK_FALSE(RegraSeAplica(im, 4, 0.f));
}
