// Leitura de regras de ataque a partir de texto JSON, e a decisao de a quem
// cada regra se aplica. Camada pura: sem arquivo, sem log, sem SDL.

#include "doctest.h"
#include "../Source/Attacks/RegrasDeAtaque.h"

TEST_CASE("Regras: le uma regra por indice com uma Motion") {
    const auto r = LerRegras(R"({
        "fase2": [
            { "indices": [0,2], "motion": {"tipo":"Tracking","atraso":0.3,"forca":3.2,"duracao":2.4} }
        ]
    })");
    CHECK(r.problemas.empty());
    REQUIRE(r.conjuntos.count("fase2") == 1);
    REQUIRE(r.conjuntos.at("fase2").size() == 1);

    const Regra& regra = r.conjuntos.at("fase2")[0];
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
        "fase2": [
            // Forca diferente entre par e impar: o jogador ve dois
            // comportamentos no mesmo anel e aprende a ler qual e qual.
            { "quando": "pares",
              "motion": {"tipo":"Tracking","atraso":0.3,"forca":3.2,"duracao":2.4} }
        ]
    })");
    CHECK(r.problemas.empty());
    REQUIRE(r.conjuntos.count("fase2") == 1);
    REQUIRE(r.conjuntos.at("fase2").size() == 1);
    CHECK(r.conjuntos.at("fase2")[0].motion.forca == doctest::Approx(3.2f));
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
    const auto r = LerRegras(R"({ "fase1": [
        { "escala": 3, "animacao": "Perseguicao",
          "investidaRepetida": {
            "ritmo": { "investida": 0.6, "pausa": 1.0, "repeticoes": 7 },
            "velocidadeNaInvestida": 900, "velocidadeNaPausa": 15 } }
    ] })");
    CHECK(r.problemas.empty());
    REQUIRE(r.conjuntos.at("fase1").size() == 1);
    const Regra& g = r.conjuntos.at("fase1")[0];

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

// ---------------------------------------------------------------------------
// A parada num ponto do caminho
//
// Serve a fase 2 do Salles, em que a lista duplamente encadeada vai ate o fim,
// PARA, e so entao volta por onde veio. Os dois campos existem porque uma pausa
// nao da para compor com o que havia: Motion e exclusiva, entao nao ha como ter
// um Path mais outra coisa que segure o projetil.
// ---------------------------------------------------------------------------

TEST_CASE("Regras: le a parada num ponto do caminho") {
    const auto r = LerRegras(R"({
      "c": [ { "motion": { "tipo": "Path", "forma": "IdaEVolta", "velocidade": 400,
                           "atraso": 0.85, "pararNoPonto": 0, "pararPor": 0.8 } } ]
    })");
    CHECK(r.problemas.empty());
    REQUIRE(r.conjuntos.count("c") == 1);
    REQUIRE(r.conjuntos.at("c").size() == 1);
    const auto& m = r.conjuntos.at("c")[0].motion;
    CHECK(m.pararNoPonto == 0);
    CHECK(m.pararPor == doctest::Approx(0.8f));
}

TEST_CASE("Regras: sem parada, os campos ficam no valor que nao para") {
    // -1 e zero sao o comportamento de TODO caminho escrito antes desta opcao
    // existir. Se o padrao fosse 0, todo caminho antigo passaria a parar no
    // primeiro ponto.
    const auto r = LerRegras(R"({
      "c": [ { "motion": { "tipo": "Path", "forma": "Reta", "velocidade": 200 } } ]
    })");
    REQUIRE(r.problemas.empty());
    const auto& m = r.conjuntos.at("c")[0].motion;
    CHECK(m.pararNoPonto == -1);
    CHECK(m.pararPor == doctest::Approx(0.0f));
}

TEST_CASE("Regras: pararPor sem pararNoPonto e recusado") {
    // Meia configuracao nao da erro em jogo: o projetil simplesmente nao para, e
    // quem escreveu vai procurar o defeito no motor em vez de no arquivo.
    const auto r = LerRegras(R"({
      "c": [ { "motion": { "tipo": "Path", "forma": "Reta", "pararPor": 0.8 } } ]
    })");
    REQUIRE_FALSE(r.problemas.empty());
    CHECK(r.problemas[0].find("pararNoPonto") != std::string::npos);
    // A regra e descartada; o conjunto continua existindo, so que vazio. E a
    // convencao do leitor para toda regra invalida, nao uma excecao da parada.
    CHECK(r.conjuntos.at("c").empty());
}

TEST_CASE("Regras: pararNoPonto sem pararPor e recusado") {
    const auto r = LerRegras(R"({
      "c": [ { "motion": { "tipo": "Path", "forma": "Reta", "pararNoPonto": 1 } } ]
    })");
    REQUIRE_FALSE(r.problemas.empty());
    CHECK(r.problemas[0].find("pararPor") != std::string::npos);
    CHECK(r.conjuntos.at("c").empty());
}

TEST_CASE("Regras: parada so existe em Path") {
    // O Tracking nao percorre waypoints, entao nao ha ponto onde parar. Aceitar
    // em silencio daria uma regra que parece configurada e nao faz nada.
    const auto r = LerRegras(R"({
      "c": [ { "motion": { "tipo": "Tracking", "forca": 2.0, "duracao": 2.0,
                           "pararNoPonto": 0, "pararPor": 0.8 } } ]
    })");
    REQUIRE_FALSE(r.problemas.empty());
    CHECK(r.problemas[0].find("Path") != std::string::npos);
}

TEST_CASE("Regras: \"pausa\" do PulsoDeVelocidade nao e a parada do caminho") {
    // Os dois nomes sao parecidos e querem dizer coisas diferentes: "pausa" no
    // PulsoDeVelocidade e um MODULO DE VELOCIDADE, nao uma duracao. Este teste
    // existe para travar a separacao - se alguem unificar os nomes, quebra aqui.
    // PulsoDeVelocidade e um MODIFIER (muda o modulo), nao uma Motion.
    const auto r = LerRegras(R"({
      "c": [ { "modifiers": [ { "tipo": "PulsoDeVelocidade", "investida": 300, "pausa": 20,
                                "ritmo": { "repeticoes": 3, "investida": 0.4, "pausa": 0.3 } } ] } ]
    })");
    CHECK(r.problemas.empty());
    REQUIRE(r.conjuntos.count("c") == 1);
    REQUIRE_FALSE(r.conjuntos.at("c").empty());
    REQUIRE_FALSE(r.conjuntos.at("c")[0].modifiers.empty());
    const auto& mod = r.conjuntos.at("c")[0].modifiers[0];
    CHECK(mod.moduloPausa == doctest::Approx(20.0f));
    CHECK(mod.pararPor == doctest::Approx(0.0f));
    CHECK(mod.pararNoPonto == -1);
}

// ---------------------------------------------------------------------------
// "rotacionar": girar e da RAJADA, e nao do tipo de projetil
// ---------------------------------------------------------------------------

TEST_CASE("Regras: \"rotacionar\" e lido como true e como false") {

    // AUSENTE E DIFERENTE DE FALSE, e e o ponto do campo ser optional: "false"
    // e uma ordem ("nao gire") e ausente e silencio ("deixe como a fabrica
    // pediu"). Se fosse um bool simples, toda regra sem o campo desligaria o
    // giro de todo mundo - inclusive o da lista do Salles, que depende dele.
    const auto r = LerRegras(R"({
      "gira":     [ { "animacao": "A", "rotacionar": true  } ],
      "nao_gira": [ { "animacao": "A", "rotacionar": false } ],
      "calado":   [ { "animacao": "A" } ]
    })");

    CHECK(r.problemas.empty());
    REQUIRE(r.conjuntos.count("gira") == 1);
    REQUIRE(r.conjuntos.count("nao_gira") == 1);
    REQUIRE(r.conjuntos.count("calado") == 1);

    REQUIRE(r.conjuntos.at("gira")[0].rotacionar.has_value());
    CHECK(*r.conjuntos.at("gira")[0].rotacionar == true);

    REQUIRE(r.conjuntos.at("nao_gira")[0].rotacionar.has_value());
    CHECK(*r.conjuntos.at("nao_gira")[0].rotacionar == false);

    CHECK_FALSE(r.conjuntos.at("calado")[0].rotacionar.has_value());
}

TEST_CASE("Regras: \"rotacionar\" que nao e booleano derruba a regra") {

    // Nao aceitar 0/1 nem "true" de proposito: um arquivo que escreve 1 aqui
    // provavelmente quis dizer outra coisa, e aceitar calado esconderia isso.
    const auto r = LerRegras(R"({ "x": [ { "animacao": "A", "rotacionar": 1 } ] })");

    CHECK(r.conjuntos.at("x").empty());
    REQUIRE(r.problemas.size() == 1);
    CHECK(r.problemas[0].find("rotacionar") != std::string::npos);
}

TEST_CASE("Regras: uma regra SO com \"rotacionar\" ja faz alguma coisa") {

    // Antes deste campo, "nao faz nada" era motion, modifiers, animacao ou
    // escala. Uma regra que so desliga o giro e legitima - o projetil ja tem
    // animacao e escala de fabrica - e precisava entrar naquela lista, senao
    // seria descartada com uma mensagem que nao faz sentido.
    const auto r = LerRegras(R"({ "x": [ { "rotacionar": false } ] })");

    CHECK(r.problemas.empty());
    REQUIRE(r.conjuntos.at("x").size() == 1);
    CHECK_FALSE(*r.conjuntos.at("x")[0].rotacionar);
}

// ---------------------------------------------------------------------------
// "alvoAcorrentado": a unica condicao que olha para o mundo, e nao para a rajada
// ---------------------------------------------------------------------------

TEST_CASE("Regras: \"alvoAcorrentado\" e lido como condicao") {

    const auto r = LerRegras(R"({
      "paralelo": [
        { "animacao": "Correndo" },
        { "quando": "alvoAcorrentado",
          "motion": { "tipo": "Tracking", "atraso": 0.25, "forca": 0.9, "duracao": 3.0 } }
      ]
    })");

    CHECK(r.problemas.empty());
    REQUIRE(r.conjuntos.at("paralelo").size() == 2);
    CHECK(r.conjuntos.at("paralelo")[0].condicao == Regra::Sempre);
    CHECK(r.conjuntos.at("paralelo")[1].condicao == Regra::AlvoAcorrentado);
}

TEST_CASE("Regras: \"alvoAcorrentado\" so se aplica quando o alvo esta preso") {

    // O ESTADO ENTRA COMO PARAMETRO, e e isso que mantem esta camada pura: ela
    // nao conhece Player nem Scene e nao vai passar a conhecer. Quem pergunta
    // ao mundo e o configurator em RegrasDeAtaqueArquivo.cpp.
    Regra r;
    r.condicao = Regra::AlvoAcorrentado;

    CHECK(RegraSeAplica(r, 0, 0.5f, /*alvoAcorrentado=*/true));
    CHECK(RegraSeAplica(r, 7, 0.1f, true));
    CHECK_FALSE(RegraSeAplica(r, 0, 0.5f, /*alvoAcorrentado=*/false));
    CHECK_FALSE(RegraSeAplica(r, 7, 0.9f, false));
}

TEST_CASE("Regras: o alvo preso nao muda as outras condicoes") {

    // Era o risco obvio de acrescentar um parametro a um switch: alguem o usa
    // num ramo errado e "pares" passa a depender do estado do jogo. As outras
    // quatro condicoes tem de dar o MESMO resultado com o alvo preso e solto.
    Regra pares;   pares.condicao = Regra::Pares;
    Regra impares; impares.condicao = Regra::Impares;
    Regra sempre;  sempre.condicao = Regra::Sempre;
    Regra chance;  chance.condicao = Regra::Chance; chance.chance = 0.5f;

    for (int i = 0; i < 6; ++i) {
        CAPTURE(i);
        CHECK(RegraSeAplica(pares,   i, 0.3f, true) == RegraSeAplica(pares,   i, 0.3f, false));
        CHECK(RegraSeAplica(impares, i, 0.3f, true) == RegraSeAplica(impares, i, 0.3f, false));
        CHECK(RegraSeAplica(sempre,  i, 0.3f, true) == RegraSeAplica(sempre,  i, 0.3f, false));
        CHECK(RegraSeAplica(chance,  i, 0.3f, true) == RegraSeAplica(chance,  i, 0.3f, false));
    }
}

TEST_CASE("Regras: o padrao de RegraSeAplica e \"ninguem preso\"") {

    // Toda chamada que existia antes deste parametro continua valendo, e o que
    // ela quer dizer e "o alvo nao esta acorrentado". Se o padrao fosse true,
    // uma regra de alvo preso valeria sempre em qualquer chamador antigo.
    Regra r;
    r.condicao = Regra::AlvoAcorrentado;
    CHECK_FALSE(RegraSeAplica(r, 0, 0.5f));
}

// ---------------------------------------------------------------------------
// Reescrever: a casa da fita de Turing
// ---------------------------------------------------------------------------

TEST_CASE("Regras: le o Reescrever com os campos dele") {
    const auto r = LerRegras(R"({ "fita": [ { "animacao": "A", "modifiers": [
        {"tipo":"SlowDown","atraso":0.7,"fator":0.08},
        {"tipo":"Reescrever","atraso":0.9,"raio":24,"escreve":"B",
         "velocidade":260,"forca":1.6,"duracao":2.2} ] } ] })");

    CHECK(r.problemas.empty());
    REQUIRE(r.conjuntos.count("fita") == 1);
    const auto& regra = r.conjuntos.at("fita").at(0);
    REQUIRE(regra.modifiers.size() == 2);

    const auto& d = regra.modifiers[1];
    CHECK(d.tipo == "Reescrever");
    CHECK(d.atraso     == doctest::Approx(0.9f));
    CHECK(d.raio       == doctest::Approx(24.0f));
    CHECK(d.escreve    == "B");
    CHECK(d.velocidade == doctest::Approx(260.0f));
    CHECK(d.forca      == doctest::Approx(1.6f));
    CHECK(d.duracao    == doctest::Approx(2.2f));
}

TEST_CASE("Regras: um Reescrever que nao faria nada e recusado dizendo o que falta") {

    // Cada um destes sai CALADO em tela: a cabeca nunca acha a casa, o "b" nasce
    // parado e o Tracking nao tem o que girar, a casa muda de comportamento com
    // a cor de antes. Recusar na leitura e o que faz o erro aparecer.
    struct Caso { const char* json; const char* campo; };
    const Caso casos[] = {
        { R"({"x":[{"modifiers":[{"tipo":"Reescrever","raio":0,"escreve":"B","velocidade":260}]}]})", "raio" },
        { R"({"x":[{"modifiers":[{"tipo":"Reescrever","raio":24,"escreve":"B"}]}]})",                 "velocidade" },
        { R"({"x":[{"modifiers":[{"tipo":"Reescrever","raio":24,"velocidade":260}]}]})",              "escreve" },
    };
    for (const auto& c : casos) {
        CAPTURE(c.campo);
        const auto r = LerRegras(c.json);
        REQUIRE(r.problemas.size() == 1);
        CHECK(r.problemas[0].find(c.campo) != std::string::npos);
        CHECK(r.conjuntos.at("x").empty());
    }
}

TEST_CASE("Regras: Reescrever e um Modifier, e nao pode ir em \"motion\"") {
    const auto r = LerRegras(R"({"x":[{"motion":{"tipo":"Reescrever","raio":24,"escreve":"B","velocidade":260}}]})");
    REQUIRE(r.problemas.size() == 1);
    CHECK(r.problemas[0].find("Modifier") != std::string::npos);
}

// ---------------------------------------------------------------------------
// Promessa: a Promise do JavaScript
// ---------------------------------------------------------------------------

TEST_CASE("Regras: le a Promessa com os campos dela") {
    const auto r = LerRegras(R"({ "p": [ { "animacao": "Promessa", "modifiers": [
        {"tipo":"SlowDown","atraso":0.8,"fator":0.0},
        {"tipo":"Promessa","atraso":1.6,"resolve":0.6,"velocidade":280,"escreve":"Tag"} ] } ] })");

    CHECK(r.problemas.empty());
    const auto& d = r.conjuntos.at("p").at(0).modifiers.at(1);
    CHECK(d.tipo == "Promessa");
    CHECK(d.atraso     == doctest::Approx(1.6f));
    CHECK(d.resolve    == doctest::Approx(0.6f));
    CHECK(d.velocidade == doctest::Approx(280.0f));
    CHECK(d.escreve    == "Tag");
}

TEST_CASE("Regras: uma Promessa parada, ou com uma fracao fora de 0 a 1, e recusada") {
    struct Caso { const char* json; const char* campo; };
    const Caso casos[] = {
        { R"({"x":[{"modifiers":[{"tipo":"Promessa","atraso":1,"resolve":0.5}]}]})",                  "velocidade" },
        { R"({"x":[{"modifiers":[{"tipo":"Promessa","atraso":1,"resolve":1.5,"velocidade":280}]}]})", "resolve" },
        { R"({"x":[{"modifiers":[{"tipo":"Promessa","atraso":1,"resolve":-0.1,"velocidade":280}]}]})","resolve" },
    };
    for (const auto& c : casos) {
        CAPTURE(c.campo);
        const auto r = LerRegras(c.json);
        REQUIRE(r.problemas.size() == 1);
        CHECK(r.problemas[0].find(c.campo) != std::string::npos);
    }
}

// ---------------------------------------------------------------------------
// Amarrar: ligare, "atar"
// ---------------------------------------------------------------------------

TEST_CASE("Regras: le o Amarrar com os campos dele") {
    const auto r = LerRegras(R"({ "a": [ { "animacao": "Livre", "motion":
        {"tipo":"Amarrar","comprimento":140,"duracao":2.4,"sentido":"antihorario","escreve":"Ligada"} } ] })");

    CHECK(r.problemas.empty());
    CHECK(r.conjuntos.at("a").at(0).temMotion);
    const auto& d = r.conjuntos.at("a").at(0).motion;
    CHECK(d.tipo == "Amarrar");
    CHECK(d.comprimento == doctest::Approx(140.0f));
    CHECK(d.duracao     == doctest::Approx(2.4f));
    CHECK(d.sentido     == "antihorario");
    CHECK(d.escreve     == "Ligada");
}

TEST_CASE("Regras: sem sentido, a corda gira no horario") {
    const auto r = LerRegras(R"({"a":[{"motion":{"tipo":"Amarrar","comprimento":140,"duracao":2}}]})");
    CHECK(r.problemas.empty());
    CHECK(r.conjuntos.at("a").at(0).motion.sentido == "horario");
}

TEST_CASE("Regras: uma corda sem comprimento, sem duracao ou com sentido errado e recusada") {
    struct Caso { const char* json; const char* campo; };
    const Caso casos[] = {
        { R"({"x":[{"motion":{"tipo":"Amarrar","duracao":2}}]})",                                      "comprimento" },
        { R"({"x":[{"motion":{"tipo":"Amarrar","comprimento":0,"duracao":2}}]})",                      "comprimento" },
        { R"({"x":[{"motion":{"tipo":"Amarrar","comprimento":140}}]})",                                "duracao" },
        { R"({"x":[{"motion":{"tipo":"Amarrar","comprimento":140,"duracao":2,"sentido":"Horario"}}]})", "sentido" },
    };
    for (const auto& c : casos) {
        CAPTURE(c.campo);
        const auto r = LerRegras(c.json);
        REQUIRE(r.problemas.size() == 1);
        CHECK(r.problemas[0].find(c.campo) != std::string::npos);
    }
}

TEST_CASE("Regras: Amarrar e uma Motion, e nao pode ir em \"modifiers\"") {
    const auto r = LerRegras(R"({"x":[{"modifiers":[{"tipo":"Amarrar","comprimento":140,"duracao":2}]}]})");
    REQUIRE(r.problemas.size() == 1);
    CHECK(r.problemas[0].find("Motion") != std::string::npos);
}
