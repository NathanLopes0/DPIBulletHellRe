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

// ---------------------------------------------------------------------------
// Comentarios
// ---------------------------------------------------------------------------

TEST_CASE("Regras: o arquivo aceita comentario") {
    // Mesma razao de test_formas: o porque de cada numero precisa caber no
    // arquivo, senao mexer nele seis meses depois e adivinhacao.
    const auto r = LerRegras(R"({
        "julio_fase2": [
            // Forca diferente entre par e impar: o jogador ve dois
            // comportamentos no mesmo anel e aprende a ler qual e qual.
            { "quando": "pares",
              "motion": {"tipo":"Tracking","atraso":0.3,"forca":3.2,"duracao":2.4} }
        ]
    })");
    CHECK(r.problemas.empty());
    REQUIRE(r.conjuntos.count("julio_fase2") == 1);
    REQUIRE(r.conjuntos.at("julio_fase2").size() == 1);
    CHECK(r.conjuntos.at("julio_fase2")[0].motion.forca == doctest::Approx(3.2f));
}

TEST_CASE("Regras: texto que nao e JSON continua sendo recusado") {
    // Aceitar comentario NAO e aceitar qualquer coisa: o erro de digitacao
    // precisa continuar virando uma frase de problema em vez de passar calado.
    const auto r = LerRegras(R"({ "x": [ { "motion": {"tipo":"Tracking"} )");
    CHECK_FALSE(r.problemas.empty());
    CHECK(r.conjuntos.empty());
}

// ---------------------------------------------------------------------------
// Escala
// ---------------------------------------------------------------------------

TEST_CASE("Regras: escala e opcional e nao tem sentinela") {
    // Escala zero nao quer dizer "deixa como esta", quer dizer projetil
    // invisivel. Por isso a ausencia e representada por nullopt, e nao por zero.
    const auto r = LerRegras(R"({ "a": [ { "escala": 3 } ],
                                  "b": [ { "animacao": "Homing" } ] })");
    CHECK(r.problemas.empty());
    REQUIRE(r.conjuntos.at("a").size() == 1);
    REQUIRE(r.conjuntos.at("a")[0].escala.has_value());
    CHECK(*r.conjuntos.at("a")[0].escala == doctest::Approx(3.f));
    CHECK_FALSE(r.conjuntos.at("b")[0].escala.has_value());
}

TEST_CASE("Regras: uma regra so de escala JA faz alguma coisa") {
    // Antes, a checagem de "nao faz nada" exigia motion, modifiers ou animacao.
    const auto r = LerRegras(R"({ "a": [ { "escala": 2 } ] })");
    CHECK(r.problemas.empty());
    CHECK(r.conjuntos.at("a").size() == 1);
}

TEST_CASE("Regras: escala zero ou negativa e recusada") {
    const auto r = LerRegras(R"({ "a": [ { "escala": 0 } ],
                                  "b": [ { "escala": -1 } ] })");
    CHECK(r.problemas.size() == 2);
    CHECK(r.conjuntos.at("a").empty());
    CHECK(r.conjuntos.at("b").empty());
}

TEST_CASE("Regras: regra vazia continua sendo recusada") {
    const auto r = LerRegras(R"({ "a": [ { "quando": "pares" } ] })");
    CHECK_FALSE(r.problemas.empty());
    CHECK(r.conjuntos.at("a").empty());
}

// ---------------------------------------------------------------------------
// investidaRepetida: um ritmo, duas pecas
// ---------------------------------------------------------------------------

TEST_CASE("Investida: expande em MiraPeriodica + PulsoDeVelocidade") {
    // O equivalente em dados de AplicarInvestidaRepetida. O ritmo e escrito UMA
    // vez e alimenta as duas pecas, que e a propriedade que a receita em C++
    // existia para garantir.
    const auto r = LerRegras(R"({ "julio_fase1": [
        { "escala": 3, "animacao": "Perseguicao",
          "investidaRepetida": {
            "ritmo": { "investida": 0.6, "pausa": 1.0, "repeticoes": 7 },
            "velocidadeNaInvestida": 900, "velocidadeNaPausa": 15 } }
    ] })");
    CHECK(r.problemas.empty());
    REQUIRE(r.conjuntos.at("julio_fase1").size() == 1);
    const Regra& g = r.conjuntos.at("julio_fase1")[0];

    REQUIRE(g.temMotion);
    CHECK(g.motion.tipo == "MiraPeriodica");
    REQUIRE(g.modifiers.size() == 1);
    CHECK(g.modifiers[0].tipo == "PulsoDeVelocidade");

    // O MESMO ritmo nas duas, por construcao.
    CHECK(g.motion.ritmo.duracaoInvestida == doctest::Approx(g.modifiers[0].ritmo.duracaoInvestida));
    CHECK(g.motion.ritmo.duracaoPausa     == doctest::Approx(g.modifiers[0].ritmo.duracaoPausa));
    CHECK(g.motion.ritmo.repeticoes       == g.modifiers[0].ritmo.repeticoes);

    CHECK(g.motion.ritmo.duracaoInvestida == doctest::Approx(0.6f));
    CHECK(g.motion.ritmo.repeticoes == 7);
    CHECK(g.modifiers[0].moduloInvestida == doctest::Approx(900.f));
    CHECK(g.modifiers[0].moduloPausa == doctest::Approx(15.f));
    CHECK(*g.escala == doctest::Approx(3.f));
    CHECK(g.animacao == "Perseguicao");
}

TEST_CASE("Investida: velocidade de pausa zero e recusada") {
    // A direcao mora dentro do vetor velocidade: com modulo zero a mirada
    // seguinte nao tem o que girar e o projetil fica parado para sempre.
    const auto r = LerRegras(R"({ "a": [ { "investidaRepetida": {
        "ritmo": { "investida": 0.6, "pausa": 1.0, "repeticoes": 5 },
        "velocidadeNaInvestida": 800, "velocidadeNaPausa": 0 } } ] })");
    CHECK_FALSE(r.problemas.empty());
    CHECK(r.conjuntos.at("a").empty());
}

TEST_CASE("Investida: nao pode vir junto de uma motion") {
    // A investida JA define a motion. Aceitar as duas faria uma substituir a
    // outra em silencio, dependendo da ordem de leitura.
    const auto r = LerRegras(R"({ "a": [ {
        "motion": { "tipo": "Tracking", "atraso": 0.3, "forca": 2, "duracao": 2 },
        "investidaRepetida": {
            "ritmo": { "investida": 0.6, "pausa": 1.0, "repeticoes": 5 },
            "velocidadeNaInvestida": 800, "velocidadeNaPausa": 15 } } ] })");
    CHECK_FALSE(r.problemas.empty());
    CHECK(r.conjuntos.at("a").empty());
}

TEST_CASE("Investida: ritmo ausente e recusado") {
    const auto r = LerRegras(R"({ "a": [ { "investidaRepetida": {
        "velocidadeNaInvestida": 800, "velocidadeNaPausa": 15 } } ] })");
    CHECK_FALSE(r.problemas.empty());
    CHECK(r.conjuntos.at("a").empty());
}
