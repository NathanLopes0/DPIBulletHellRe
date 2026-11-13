// Leitura de fases de ataque a partir de texto JSON, e a validacao das transicoes.

#include "doctest.h"

#include <stdexcept>
#include <string>
#include "../Source/Attacks/FasesDeAtaque.h"
#include "../Source/Regras/Tabela.h"

// Um conjunto minimo que fecha, para servir de base aos casos que mexem em uma coisa de cada vez.
static const char* kConjuntoValido = R"({
  "StateOne": {
    "duracao": 17, "proximo": "StateFinal",
    "movimento": { "tipo": "RandomWander", "a": 4, "b": 100 },
    "ataques": [
      { "estrategia": "CircleSpreadAttack", "projetil": "Arduino",
        "projeteis": 36, "velocidade": 180, "cooldown": 2.4 }
    ]
  },
  "StateFinal": {
    "duracao": 17, "proximo": "StateOne",
    "movimento": { "tipo": "GoToCenter" },
    "ataques": [
      { "estrategia": "CircleSpreadAttack", "projetil": "Arduino",
        "projeteis": 18, "velocidade": 120, "cooldown": 1.4 }
    ]
  }
})";

// Leitura.

TEST_CASE("Fases: le um conjunto completo") {
    const auto r = LerFases(kConjuntoValido);
    CHECK(r.problemas.empty());
    REQUIRE(!r.fases.empty());
    REQUIRE(r.fases.size() == 2);

    // O mapa de fases e ordenado por nome, entao StateFinal vem antes de StateOne.
    const auto& fases = r.fases;
    const DescricaoDeFase* uma = nullptr;
    for (const auto& f : fases) if (f.nome == "StateOne") uma = &f;
    REQUIRE(uma != nullptr);

    CHECK(uma->duracao == doctest::Approx(17.f));
    CHECK(uma->proximo == "StateFinal");
    CHECK(uma->movimento.tipo == "RandomWander");
    CHECK(uma->movimento.a == doctest::Approx(4.f));
    CHECK(uma->movimento.b == doctest::Approx(100.f));
    REQUIRE(uma->ataques.size() == 1);
    CHECK(uma->ataques[0].estrategia == "CircleSpreadAttack");
    CHECK(uma->ataques[0].projetil == "Arduino");
    CHECK(uma->ataques[0].cooldown == doctest::Approx(2.4f));
}

TEST_CASE("Fases: o campo ausente fica AUSENTE, nao zero") {
    // O ponto central do formato.
    const auto r = LerFases(kConjuntoValido);
    REQUIRE(!r.fases.empty());
    const auto& fases = r.fases;
    const DescricaoDeAtaque* a = nullptr;
    for (const auto& f : fases) if (f.nome == "StateOne") a = &f.ataques[0];
    REQUIRE(a != nullptr);

    CHECK(a->projeteis.has_value());
    CHECK(a->velocidade.has_value());
    CHECK_FALSE(a->angulo.has_value());
    CHECK_FALSE(a->anguloCentral.has_value());
    CHECK_FALSE(a->intervalo.has_value());
}

TEST_CASE("Fases: le os valores opcionais quando eles estao lá") {
    const auto r = LerFases(R"({
 "StateOne": { "proximo": "StateFinal", "ataques": [
        { "estrategia": "CircleSpreadAttack", "projetil": "Capivara",
          "projeteis": 3, "velocidade": 340, "angulo": 40,
          "anguloCentral": 90, "intervalo": 0.02, "cooldown": 0.8 } ] },
                "StateFinal": { "ataques": [
        { "estrategia": "CircleSpreadAttack", "projetil": "Capivara", "cooldown": 1 } ] }

    })");
    CHECK(r.problemas.empty());
    const auto& a = r.fases.at(1).ataques[0];   // StateOne
    REQUIRE(a.angulo.has_value());
    CHECK(*a.angulo == doctest::Approx(40.f));
    REQUIRE(a.anguloCentral.has_value());
    CHECK(*a.anguloCentral == doctest::Approx(90.f));
    REQUIRE(a.intervalo.has_value());
    CHECK(*a.intervalo == doctest::Approx(0.02f));
}

TEST_CASE("Fases: regras por nome e regras em linha") {
    const auto r = LerFases(R"({
      "StateOne": { "proximo": "StateFinal", "ataques": [
        { "estrategia": "CircleSpreadAttack", "projetil": "P", "cooldown": 1,
          "regras": "julio_fase2" } ] },
      "StateFinal": { "ataques": [
        { "estrategia": "CircleSpreadAttack", "projetil": "P", "cooldown": 1,
          "regras": [ { "quando": "pares",
                        "motion": {"tipo":"Tracking","atraso":0.3,"forca":3.2,"duracao":2.4} } ] } ] }

    })");
    CHECK(r.problemas.empty());
    const auto& fases = r.fases;

    const DescricaoDeAtaque* porNome = nullptr;
    const DescricaoDeAtaque* emLinha = nullptr;
    for (const auto& f : fases) {
        if (f.nome == "StateOne")   porNome = &f.ataques[0];
        if (f.nome == "StateFinal") emLinha = &f.ataques[0];
    }
    REQUIRE(porNome != nullptr);
    REQUIRE(emLinha != nullptr);

    CHECK(porNome->regrasNome == "julio_fase2");
    CHECK_FALSE(porNome->temRegrasEmLinha);

    CHECK(emLinha->temRegrasEmLinha);
    REQUIRE(emLinha->regras.size() == 1);
    CHECK(emLinha->regras[0].condicao == Regra::Pares);
    CHECK(emLinha->regras[0].motion.forca == doctest::Approx(3.2f));
}

TEST_CASE("Fases: o arquivo aceita comentario") {
    const auto r = LerFases(R"({
      // 36 projeteis: o anel precisa ser denso para nao dar para atravessar.
      "StateOne":   { "proximo": "StateFinal", "ataques": [
        { "estrategia": "CircleSpreadAttack", "projetil": "P", "cooldown": 2.4 } ] },
      "StateFinal": { "ataques": [
        { "estrategia": "CircleSpreadAttack", "projetil": "P", "cooldown": 1.4 } ] }

    })");
    CHECK(r.problemas.empty());
    CHECK(r.fases.size() == 2);
}

// O que e recusado.

TEST_CASE("Fases: estrategia inexistente e recusada") {
    std::vector<std::string> ps;
    const auto r = LerFases(R"({ "StateOne": { "ataques": [
      { "estrategia": "AtaqueQueNaoExiste", "projetil": "P", "cooldown": 1 } ] }
})");
    CHECK_FALSE(r.problemas.empty());
    // A fase perdeu o unico ataque, entao a fase inteira caiu.
    CHECK(r.fases.empty());
}

TEST_CASE("Fases: ataque sem projetil e recusado") {
    const auto r = LerFases(R"({ "StateOne": { "ataques": [
      { "estrategia": "CircleSpreadAttack", "cooldown": 1 } ] }
})");
    CHECK_FALSE(r.problemas.empty());
    CHECK(r.fases.empty());
}

TEST_CASE("Fases: cooldown zero e recusado") {
    // Cooldown zero dispara uma rajada por quadro e enche o pool em menos de um
    // segundo.
    const auto r = LerFases(R"({ "StateOne": { "ataques": [
      { "estrategia": "CircleSpreadAttack", "projetil": "P", "cooldown": 0 } ] }
})");
    CHECK_FALSE(r.problemas.empty());
    CHECK(r.fases.empty());
}

TEST_CASE("Fases: projeteis abaixo de 1 e recusado") {
    const auto r = LerFases(R"({ "StateOne": { "ataques": [
      { "estrategia": "CircleSpreadAttack", "projetil": "P", "projeteis": 0, "cooldown": 1 } ] }
})");
    CHECK_FALSE(r.problemas.empty());
    CHECK(r.fases.empty());
}

TEST_CASE("Fases: nome de fase inventado e recusado") {
    // A FSM procura os estados por texto.
    const auto r = LerFases(R"({ "FaseDoMeio": { "ataques": [
      { "estrategia": "CircleSpreadAttack", "projetil": "P", "cooldown": 1 } ] }
})");
    CHECK_FALSE(r.problemas.empty());
    CHECK(r.fases.empty());
}

TEST_CASE("Fases: movimento inexistente cai para GoToCenter e avisa") {
    const auto r = LerFases(R"({ "StateOne": {
      "movimento": { "tipo": "VoarEmCirculos" },
      "ataques": [ { "estrategia": "CircleSpreadAttack", "projetil": "P", "cooldown": 1 } ] }
})");
    CHECK_FALSE(r.problemas.empty());
    REQUIRE(r.fases.size() == 1);
    CHECK(r.fases[0].movimento.tipo == "GoToCenter");
}

TEST_CASE("Fases: um ataque ruim nao leva os bons embora") {
    const auto r = LerFases(R"({ "StateOne": { "ataques": [
      { "estrategia": "CircleSpreadAttack", "projetil": "P", "cooldown": 1 },
      { "estrategia": "NaoExiste",    "projetil": "P", "cooldown": 1 },
      { "estrategia": "WaveAttack",   "projetil": "P", "cooldown": 2 } ] }
})");
    CHECK(r.problemas.size() == 1);
    REQUIRE(r.fases.size() == 1);
    CHECK(r.fases[0].ataques.size() == 2);
}

TEST_CASE("Fases: texto que nao e JSON vira problema, nao excecao") {
    const auto r = LerFases(R"({ "StateOne": )");
    CHECK_FALSE(r.problemas.empty());
    CHECK(r.fases.empty());
}

// Os nomes que existem.

TEST_CASE("Fases: as listas de nomes validos") {
    CHECK(EstrategiaExiste("CircleSpreadAttack"));
    CHECK(EstrategiaExiste("WaveAttack"));
    CHECK(EstrategiaExiste("ConsultaAttack"));
    CHECK_FALSE(EstrategiaExiste("circlespreadattack"));   // sensivel a caixa
    // Tiradas por nunca terem sido usadas por fase nenhuma.
    CHECK_FALSE(EstrategiaExiste("AngledAttack"));
    CHECK_FALSE(EstrategiaExiste("BaloonAttack"));
    CHECK_FALSE(EstrategiaExiste("LaserAttack"));

    CHECK(MovimentoExiste("RandomWander"));
    CHECK(MovimentoExiste("HoverAbovePlayer"));
    CHECK(MovimentoExiste("GoToCenter"));
    CHECK_FALSE(MovimentoExiste("Parado"));

    CHECK(NomeDeFaseExiste("StateOne"));
    CHECK(NomeDeFaseExiste("StateFinal"));
    CHECK_FALSE(NomeDeFaseExiste("StateFive"));
}

// Transicoes.

TEST_CASE("Transicoes: um conjunto que fecha nao tem problema") {
    const auto r = LerFases(kConjuntoValido);
    CHECK(ValidarTransicoes(r.fases, "ricardo").empty());
}

TEST_CASE("Transicoes: proximo com erro de digitacao e apontado") {
    // A armadilha que o formato de arquivo multiplicaria.
    std::vector<DescricaoDeFase> fases(2);
    fases[0].nome = "StateOne";   fases[0].proximo = "StateTow";   // trocado
    fases[1].nome = "StateFinal"; fases[1].proximo = "StateOne";
    const auto ps = ValidarTransicoes(fases, "x");
    REQUIRE(ps.size() == 1);
    CHECK(ps[0].find("StateTow") != std::string::npos);
}

TEST_CASE("Transicoes: a falta de StateFinal e apontada") {
    // Sem o estado, o chefe congela.
    std::vector<DescricaoDeFase> fases(1);
    fases[0].nome = "StateOne";
    const auto ps = ValidarTransicoes(fases, "x");
    REQUIRE(ps.size() == 1);
    CHECK(ps[0].find("StateFinal") != std::string::npos);
}

TEST_CASE("Transicoes: a falta de StateOne e apontada") {
    std::vector<DescricaoDeFase> fases(1);
    fases[0].nome = "StateFinal";
    const auto ps = ValidarTransicoes(fases, "x");
    REQUIRE(ps.size() == 1);
    CHECK(ps[0].find("StateOne") != std::string::npos);
}

TEST_CASE("Transicoes: proximo vazio e legitimo") {
    // E como a terceira fase diz "a batalha e decidida aqui".
    std::vector<DescricaoDeFase> fases(2);
    fases[0].nome = "StateOne";   fases[0].proximo = "";
    fases[1].nome = "StateFinal"; fases[1].proximo = "";
    CHECK(ValidarTransicoes(fases, "x").empty());
}

TEST_CASE("Transicoes: conjunto vazio e apontado") {
    CHECK_FALSE(ValidarTransicoes({}, "x").empty());
}

// Cooldown derivado do ritmo.

TEST_CASE("Cooldown: derivado do total do ritmo da investida") {
    // O que o C++ fazia com kRitmoInvestida.DuracaoTotal()/repeticoes.
    const auto r = LerFases(R"({
  "StateOne": { "proximo": "StateFinal", "ataques": [
      { "estrategia": "CircleSpreadAttack", "projetil": "Dados",
        "cooldown": { "doRitmo": "total", "dividirPor": 7 },
        "regras": [ { "investidaRepetida": {
            "ritmo": { "investida": 0.6, "pausa": 1.0, "repeticoes": 7 },
            "velocidadeNaInvestida": 900, "velocidadeNaPausa": 15 } } ] } ] },
    "StateFinal": { "ataques": [
      { "estrategia": "CircleSpreadAttack", "projetil": "Dados", "cooldown": 1 } ] }
})");
    CHECK(r.problemas.empty());
    const auto& fases = r.fases;
    const DescricaoDeAtaque* a = nullptr;
    for (const auto& f : fases) if (f.nome == "StateOne") a = &f.ataques[0];
    REQUIRE(a != nullptr);
    CHECK(a->cooldown == doctest::Approx(10.2f / 7.0f));
}

TEST_CASE("Cooldown: ciclo, e o campo mais") {
    // Ciclo() = investida + pausa = 1.6; 1.6 + 0.5 = 2.1.
    const auto r = LerFases(R"({
  "StateOne": { "proximo": "StateFinal", "ataques": [
      { "estrategia": "CircleSpreadAttack", "projetil": "P",
        "cooldown": { "doRitmo": "ciclo", "mais": 0.5 },
        "regras": [ { "investidaRepetida": {
            "ritmo": { "investida": 0.6, "pausa": 1.0, "repeticoes": 5 },
            "velocidadeNaInvestida": 800, "velocidadeNaPausa": 15 } } ] } ] },
    "StateFinal": { "ataques": [
      { "estrategia": "CircleSpreadAttack", "projetil": "P", "cooldown": 1 } ] }
})");
    CHECK(r.problemas.empty());
    for (const auto& f : r.fases)
        if (f.nome == "StateOne") CHECK(f.ataques[0].cooldown == doctest::Approx(2.1f));
}

TEST_CASE("Cooldown: pedir o ritmo sem ter investida em linha e recusado") {
    // Regras apontadas por nome vivem em regras.json.
    const auto r = LerFases(R"({ "StateOne": { "ataques": [
      { "estrategia": "CircleSpreadAttack", "projetil": "P",
        "cooldown": { "doRitmo": "total" },
        "regras": "julio_fase2" } ] }
})");
    CHECK_FALSE(r.problemas.empty());
    CHECK(r.fases.empty());
}

TEST_CASE("Cooldown: doRitmo com nome errado e recusado") {
    const auto r = LerFases(R"({ "StateOne": { "ataques": [
      { "estrategia": "CircleSpreadAttack", "projetil": "P",
        "cooldown": { "doRitmo": "duracao" },
        "regras": [ { "investidaRepetida": {
            "ritmo": { "investida": 0.6, "pausa": 1.0, "repeticoes": 5 },
            "velocidadeNaInvestida": 800, "velocidadeNaPausa": 15 } } ] } ] }
})");
    CHECK_FALSE(r.problemas.empty());
    CHECK(r.fases.empty());
}

// ---- Consulta ----

namespace {
    /// Uma ConsultaAttack com o bloco "consulta" que o teste quiser.
    std::string ComConsulta(const std::string& bloco) {
        return R"({ "StateOne": { "ataques": [
          { "estrategia": "ConsultaAttack", "projetil": "P", "cooldown": 1,
            "consulta": )" + bloco + R"( } ] }
})";
    }
}

TEST_CASE("Consulta: um indice fora da tabela e recusado") {

    // O defeito que isto fecha.
    CHECK_FALSE(LerFases(ComConsulta(R"({ "eixo": "Linha",  "linhas": 1, "indice": 2 })")).problemas.empty());
    CHECK_FALSE(LerFases(ComConsulta(R"({ "eixo": "Coluna", "colunas": 1, "indice": 3 })")).problemas.empty());
    CHECK_FALSE(LerFases(ComConsulta(R"({ "eixo": "Linha",  "linhas": 4, "indice": 4 })")).problemas.empty());
}

TEST_CASE("Consulta: o indice e conferido contra o EIXO, nao contra o outro") {

    CHECK_FALSE(LerFases(ComConsulta(
        R"({ "eixo": "Linha", "linhas": 4, "colunas": 5, "indice": 4 })")).problemas.empty());

    CHECK(LerFases(ComConsulta(
        R"({ "eixo": "Coluna", "linhas": 4, "colunas": 5, "indice": 4 })")).problemas.empty());
}

TEST_CASE("Consulta: sem \"linhas\" o padrao da Tabela e que vale") {

    // Omitir o numero de faixas e legitimo - o padrao de Tabela::Forma vale.
    const Tabela::Forma padrao{};
    CHECK(LerFases(ComConsulta(
        R"({ "eixo": "Linha", "indice": )" + std::to_string(padrao.linhas - 1) + " }")).problemas.empty());
    CHECK_FALSE(LerFases(ComConsulta(
        R"({ "eixo": "Linha", "indice": )" + std::to_string(padrao.linhas) + " }")).problemas.empty());
}

TEST_CASE("Consulta: o indice pode ser omitido - quem escolhe e o chefe") {

    CHECK(LerFases(ComConsulta(R"({ "eixo": "Linha", "linhas": 4 })")).problemas.empty());
}

TEST_CASE("Consulta: aviso negativo e recusado") {

    // Aviso negativo e a varredura disparando antes de aparecer.
    CHECK_FALSE(LerFases(ComConsulta(R"({ "eixo": "Linha", "aviso": -0.5 })")).problemas.empty());
    CHECK(LerFases(ComConsulta(R"({ "eixo": "Linha", "aviso": 0 })")).problemas.empty());
}

// ---- A fase que se divide em dois corpos ----

namespace {

    /// Um conjunto minimo valido, com a fase 3 declarando um clone.
    std::string ComClone(const std::string& extraNaTres = "") {
        return R"({
          "StateOne":   { "duracao": 10, "proximo": "StateTwo",
                          "ataques": [ { "estrategia": "WaveAttack", "projetil": "P" } ] },
          "StateTwo":   { "duracao": 10, "proximo": "StateThree",
                          "ataques": [ { "estrategia": "CircleSpreadAttack", "projetil": "P" } ] },
          "StateThree": { "duracao": 10, "proximo": "",
                          "ataquesDe": "StateOne",
                          "clone": { "ataquesDe": "StateTwo" })" + extraNaTres + R"( },
          "StateFinal": { "duracao": 10, "proximo": "StateOne",
                          "ataques": [ { "estrategia": "WaveAttack", "projetil": "P" } ] }
        })";
    }
}

TEST_CASE("Fases: uma fase pode usar os ataques de outra, e declarar um clone") {

    const auto lidas = LerFases(ComClone());

    CHECK(lidas.problemas.empty());
    REQUIRE(lidas.fases.size() == 4);

    const DescricaoDeFase* tres = nullptr;
    for (const auto& f : lidas.fases) if (f.nome == "StateThree") tres = &f;
    REQUIRE(tres != nullptr);

    CHECK(tres->ataquesDe == "StateOne");
    CHECK(tres->clone.existe);
    CHECK(tres->clone.ataquesDe == "StateTwo");

    // A fase que empresta nao ganha a lista aqui.
    CHECK(tres->ataques.empty());
}

TEST_CASE("Fases: uma fase sem clone nao inventa um") {

    const auto lidas = LerFases(R"({
      "StateOne":   { "proximo": "StateFinal",
                      "ataques": [ { "estrategia": "WaveAttack", "projetil": "P" } ] },
      "StateFinal": { "proximo": "", 
                      "ataques": [ { "estrategia": "WaveAttack", "projetil": "P" } ] }
    })");

    REQUIRE(lidas.fases.size() == 2);
    for (const auto& f : lidas.fases) {
        CAPTURE(f.nome);
        CHECK_FALSE(f.clone.existe);
        CHECK(f.ataquesDe.empty());
    }
}

TEST_CASE("Fases: \"ataques\" E \"ataquesDe\" juntos sao recusados") {

    const auto lidas = LerFases(R"({
      "StateOne":   { "proximo": "StateFinal",
                      "ataques": [ { "estrategia": "WaveAttack", "projetil": "P" } ] },
      "StateFinal": { "proximo": "", "ataquesDe": "StateOne",
                      "ataques": [ { "estrategia": "WaveAttack", "projetil": "P" } ] }
    })");

    bool reclamou = false;
    for (const auto& p : lidas.problemas) {
        if (p.find("ataquesDe") != std::string::npos) reclamou = true;
    }
    CHECK(reclamou);
}

TEST_CASE("Fases: um clone sem \"ataquesDe\" e recusado") {

    const auto lidas = LerFases(R"({
      "StateOne":   { "proximo": "StateFinal",
                      "ataques": [ { "estrategia": "WaveAttack", "projetil": "P" } ],
                      "clone": { } },
      "StateFinal": { "proximo": "",
                      "ataques": [ { "estrategia": "WaveAttack", "projetil": "P" } ] }
    })");

    CHECK_FALSE(lidas.problemas.empty());
}

TEST_CASE("Fases: apontar para uma fase que nao existe e denunciado") {

    // Um nome errado aqui falharia em silencio.
    const auto lidas = LerFases(ComClone());
    auto fases = lidas.fases;
    for (auto& f : fases) {
        if (f.nome == "StateThree") { f.clone.ataquesDe = "StateQuinta"; }
    }

    const auto problemas = ValidarTransicoes(fases, "teste");

    bool citou = false;
    for (const auto& p : problemas) {
        if (p.find("StateQuinta") != std::string::npos) citou = true;
    }
    CHECK(citou);
}

TEST_CASE("Fases: uma fase que empresta de si mesma e recusada") {

    // Seria uma fase sem ataques: a resolucao nao tem de onde copiar.
    auto fases = LerFases(ComClone()).fases;
    for (auto& f : fases) if (f.nome == "StateThree") f.ataquesDe = "StateThree";

    const auto problemas = ValidarTransicoes(fases, "teste");

    bool citou = false;
    for (const auto& p : problemas) {
        if (p.find("ela mesma") != std::string::npos) citou = true;
    }
    CHECK(citou);
}

TEST_CASE("Fases: corrente de \"ataquesDe\" e recusada") {

    // Permitir A -> B -> C exigiria detectar ciclo, e o ganho seria nenhum.
    auto fases = LerFases(ComClone()).fases;
    for (auto& f : fases) {
        if (f.nome == "StateThree") f.ataquesDe = "StateFinal";
        if (f.nome == "StateFinal") f.ataquesDe = "StateOne";
    }

    const auto problemas = ValidarTransicoes(fases, "teste");

    bool citou = false;
    for (const auto& p : problemas) {
        if (p.find("por sua vez") != std::string::npos) citou = true;
    }
    CHECK(citou);
}

// O anuncio do placar.

TEST_CASE("Fases: o anuncio da fase e lido como texto, e sem ele a fase nao anuncia") {

    // UTF-8 inteiro, porque e o que o placar desenha: "aⁿbⁿ" com o sobrescrito de verdade.
    const std::string aNbN = "a\xE2\x81\xBF" "b\xE2\x81\xBF";
    const auto lidas = LerFases(R"({
      "StateOne":   { "proximo": "StateFinal", "anuncio": ")" + aNbN + R"(",
                      "ataques": [ { "estrategia": "WaveAttack", "projetil": "P" } ] },
      "StateFinal": { "proximo": "",
                      "ataques": [ { "estrategia": "WaveAttack", "projetil": "P" } ] }
    })");

    CHECK(lidas.problemas.empty());
    REQUIRE(lidas.fases.size() == 2);
    for (const auto& f : lidas.fases) {
        CAPTURE(f.nome);
        if (f.nome == "StateOne")   CHECK(f.anuncio == aNbN);
        if (f.nome == "StateFinal") CHECK(f.anuncio.empty());
    }
}

TEST_CASE("Fases: um anuncio que nao e texto e recusado, e nao lido como vazio") {

    // Lido como vazio, ele viraria um placar que apaga sem aviso.
    const auto lidas = LerFases(R"({
      "StateOne":   { "proximo": "StateFinal", "anuncio": 42,
                      "ataques": [ { "estrategia": "WaveAttack", "projetil": "P" } ] },
      "StateFinal": { "proximo": "",
                      "ataques": [ { "estrategia": "WaveAttack", "projetil": "P" } ] }
    })");

    bool reclamou = false;
    for (const auto& p : lidas.problemas) {
        if (p.find("anuncio") != std::string::npos) reclamou = true;
    }
    CHECK(reclamou);
}

// O Programa de Intercambio.

namespace {
    /// Duas fases que fecham, com o "intercambio" que o caso pedir na primeira.
    std::string ComIntercambio(const std::string& intercambio) {
        return R"({
          "StateOne":   { "proximo": "StateFinal", "intercambio": )" + intercambio + R"(,
                          "ataques": [ { "estrategia": "WaveAttack", "projetil": "P" } ] },
          "StateFinal": { "proximo": "",
                          "ataques": [ { "estrategia": "WaveAttack", "projetil": "P" } ] }
        })";
    }
}

TEST_CASE("Fases: o intercambio e lido, e sem ele a fase nao troca ninguem") {
    const auto lidas = LerFases(ComIntercambio(R"({ "aCada": 8, "aviso": 1.5 })"));

    CHECK(lidas.problemas.empty());
    REQUIRE(lidas.fases.size() == 2);
    for (const auto& f : lidas.fases) {
        CAPTURE(f.nome);
        if (f.nome == "StateOne") {
            REQUIRE(f.intercambio.has_value());
            CHECK(f.intercambio->aCada == doctest::Approx(8.0f));
            CHECK(f.intercambio->aviso == doctest::Approx(1.5f));
        }
        if (f.nome == "StateFinal") CHECK_FALSE(f.intercambio.has_value());
    }
}

TEST_CASE("Fases: um intercambio que nao trocaria ninguem e recusado") {

    // Cada um destes passaria calado em jogo.
    for (const char* ruim : { R"({ "aCada": 0, "aviso": 0 })",
                              R"({ "aCada": 8, "aviso": 8 })",
                              R"({ "aCada": 8, "aviso": -1 })",
                              R"({ "aCada": 8 })",
                              R"(8)" }) {
        CAPTURE(ruim);
        const auto lidas = LerFases(ComIntercambio(ruim));
        bool reclamou = false;
        for (const auto& p : lidas.problemas) {
            if (p.find("intercambio") != std::string::npos) reclamou = true;
        }
        CHECK(reclamou);
    }
}

// A caixa do HTML.

namespace {
    std::string ComAtaque(const std::string& ataque) {
        return R"({
          "StateOne":   { "proximo": "StateFinal", "ataques": [ )" + ataque + R"( ] },
          "StateFinal": { "proximo": "",
                          "ataques": [ { "estrategia": "WaveAttack", "projetil": "P" } ] }
        })";
    }
}

TEST_CASE("Fases: a CaixaAttack existe e o bloco dela e opcional") {

    const auto sem = LerFases(ComAtaque(R"({ "estrategia": "CaixaAttack", "projetil": "P" })"));
    CHECK(sem.problemas.empty());

    const auto com = LerFases(ComAtaque(R"({ "estrategia": "CaixaAttack", "projetil": "P",
                                             "caixa": { "proporcao": 2, "meiaAlturaInicial": 80 } })"));
    CHECK(com.problemas.empty());
    for (const auto& f : com.fases) {
        if (f.nome != "StateOne") continue;
        REQUIRE(f.ataques.size() == 1);
        REQUIRE(f.ataques[0].caixa.has_value());
        CHECK(*f.ataques[0].caixa->proporcao == doctest::Approx(2.0f));
        CHECK(*f.ataques[0].caixa->meiaAlturaInicial == doctest::Approx(80.0f));
    }
}

TEST_CASE("Fases: uma caixa de lado zero e recusada, e o bloco em outra estrategia e avisado") {

    // Zero nao e uma caixa estranha, e caixa nenhuma: o ataque nao dispararia.
    const auto zero = LerFases(ComAtaque(R"({ "estrategia": "CaixaAttack", "projetil": "P",
                                              "caixa": { "proporcao": 0 } })"));
    bool reclamou = false;
    for (const auto& p : zero.problemas) if (p.find("caixa") != std::string::npos) reclamou = true;
    CHECK(reclamou);

    // Num WaveAttack o bloco nao seria lido por ninguem.
    const auto fora = LerFases(ComAtaque(R"({ "estrategia": "WaveAttack", "projetil": "P",
                                              "caixa": { "proporcao": 2 } })"));
    bool avisou = false;
    for (const auto& p : fora.problemas) if (p.find("so a CaixaAttack") != std::string::npos) avisou = true;
    CHECK(avisou);
}

// O flexbox do CSS.

TEST_CASE("Fases: a FlexAttack saiu, e um arquivo que ainda a pede e avisado") {
    const auto lidas = LerFases(ComAtaque(R"({ "estrategia": "FlexAttack", "projetil": "P" })"));
    CHECK_FALSE(lidas.problemas.empty());
}

TEST_CASE("Fases: o primeiro disparo de uma janela - 1 s na que abre a fase, 0,25 s depois nas outras") {
    // E a conta que o boss faz quadro a quadro.
    CHECK(PrimeiroDisparoNaJanela(0.0f) == doctest::Approx(kPrimeiroDisparoDaFase));
    CHECK(PrimeiroDisparoNaJanela(0.75f) == doctest::Approx(0.75f + kPrimeiroDisparoDaJanela));
    CHECK(PrimeiroDisparoNaJanela(3.75f) == doctest::Approx(3.75f + kPrimeiroDisparoDaJanela));
    CHECK(kPrimeiroDisparoDaFase == doctest::Approx(1.0f));
    CHECK(kPrimeiroDisparoDaJanela == doctest::Approx(0.25f));
}

TEST_CASE("Fases: um ataque pode ter gatilho, e so um gatilho que algum chefe puxa") {
    const auto ok = LerFases(ComAtaque(R"({ "gatilho": "notificacao", "estrategia": "WaveAttack", "projetil": "P" })"));
    CHECK(ok.problemas.empty());
    bool viu = false;
    for (const auto& f : ok.fases) {
        if (f.nome != "StateOne") continue;
        REQUIRE(f.ataques.size() == 1);
        CHECK(f.ataques[0].gatilho == "notificacao");
        viu = true;
    }
    CHECK(viu);

    // Sem gatilho, o relogio.
    const auto sem = LerFases(ComAtaque(R"({ "estrategia": "WaveAttack", "projetil": "P" })"));
    for (const auto& f : sem.fases) if (f.nome == "StateOne") CHECK(f.ataques[0].gatilho.empty());

    // Um gatilho que ninguem puxa e um ataque que nunca aparece: recusado.
    for (const char* g : { R"("gatilho": "trovao")", R"("gatilho": 2)" }) {
        CAPTURE(g);
        const auto r = LerFases(ComAtaque(std::string("{ ") + g + R"(, "estrategia": "WaveAttack", "projetil": "P" })"));
        bool reclamou = false;
        for (const auto& p : r.problemas) if (p.find("gatilho") != std::string::npos) reclamou = true;
        CHECK(reclamou);
        for (const auto& f : r.fases) if (f.nome == "StateOne") CHECK(f.ataques.empty());
    }
}

TEST_CASE("Fases: a TokenAttack existe e o bloco dela e opcional") {
    const auto sem = LerFases(ComAtaque(R"({ "estrategia": "TokenAttack", "projetil": "P" })"));
    CHECK(sem.problemas.empty());

    const auto com = LerFases(ComAtaque(R"({ "estrategia": "TokenAttack", "projetil": "P",
        "tokens": { "temperatura": 0.8, "candidatos": 9, "abertura": 120, "frase": ["o", "mo"] } })"));
    CHECK(com.problemas.empty());
    bool viu = false;
    for (const auto& f : com.fases) {
        if (f.nome != "StateOne") continue;
        REQUIRE(f.ataques.size() == 1);
        REQUIRE(f.ataques[0].tokens.has_value());
        const auto& t = *f.ataques[0].tokens;
        CHECK(*t.temperatura == doctest::Approx(0.8f));
        CHECK(*t.candidatos == 9);
        CHECK(*t.abertura == doctest::Approx(120.0f));
        REQUIRE(t.frase.size() == 2);
        CHECK(t.frase[1] == "mo");
        viu = true;
    }
    CHECK(viu);
}

TEST_CASE("Fases: tokens com temperatura ou abertura negativas, candidatos que nao sao inteiros ou frase quebrada sao recusados") {
    for (const char* bloco : { R"("tokens": { "temperatura": -0.1 })", R"("tokens": { "abertura": -10 })",
                               R"("tokens": { "candidatos": 0 })", R"("tokens": { "candidatos": 2.5 })",
                               R"("tokens": { "frase": "o mo" })", R"("tokens": { "frase": ["o", 3] })" }) {
        CAPTURE(bloco);
        const auto r = LerFases(ComAtaque(std::string(R"({ "estrategia": "TokenAttack", "projetil": "P", )")
                                          + bloco + " }"));
        bool reclamou = false;
        for (const auto& p : r.problemas) if (p.find("tokens") != std::string::npos) reclamou = true;
        CHECK(reclamou);
        for (const auto& f : r.fases) if (f.nome == "StateOne") CHECK(f.ataques.empty());
    }
    // Temperatura zero vale: e a escolha gulosa.
    const auto gulosa = LerFases(ComAtaque(R"({ "estrategia": "TokenAttack", "projetil": "P", "tokens": { "temperatura": 0 } })"));
    CHECK(gulosa.problemas.empty());
}

TEST_CASE("Fases: a PalavraAttack existe, e o bloco dela e opcional") {
    const auto sem = LerFases(ComAtaque(R"({ "estrategia": "PalavraAttack", "projetil": "P", "projeteis": 16 })"));
    CHECK(sem.problemas.empty());

    const auto com = LerFases(ComAtaque(R"({ "estrategia": "PalavraAttack", "projetil": "P", "projeteis": 8,
        "palavra": { "altura": 390, "zero": "Z", "um": "U", "atravessa": "todos", "valor": "1011 0010" } })"));
    CHECK(com.problemas.empty());
    bool viu = false;
    for (const auto& f : com.fases) {
        if (f.nome != "StateOne") continue;
        REQUIRE(f.ataques.size() == 1);
        REQUIRE(f.ataques[0].palavra.has_value());
        const auto& p = *f.ataques[0].palavra;
        CHECK(*p.altura == doctest::Approx(390.0f));
        CHECK(p.zero == "Z");
        CHECK(p.um == "U");
        CHECK(p.todosAtravessam);
        REQUIRE(p.valor);
        CHECK(*p.valor == 0xB2u);   // escrito como se le: o primeiro digito e o bit mais alto
        viu = true;
    }
    CHECK(viu);
}

TEST_CASE("Fases: uma palavra com valor do tamanho errado, digito que nao e bit ou travessia inventada e recusada") {
    for (const char* bloco : { R"("palavra": { "valor": "1011 001" })",      // um digito a menos
                               R"("palavra": { "valor": "1011 00101" })",    // um a mais
                               R"("palavra": { "valor": "1021 0010" })",
                               R"("palavra": { "valor": 178 })",
                               R"("palavra": { "atravessa": "uns" })" }) {
        CAPTURE(bloco);
        const auto r = LerFases(ComAtaque(std::string(R"({ "estrategia": "PalavraAttack", "projetil": "P", "projeteis": 8, )")
                                          + bloco + " }"));
        bool reclamou = false;
        for (const auto& p : r.problemas) if (p.find("palavra") != std::string::npos) reclamou = true;
        CHECK(reclamou);
        for (const auto& f : r.fases) if (f.nome == "StateOne") CHECK(f.ataques.empty());
    }
}

TEST_CASE("Fases: a PontosAttack precisa de um posto que existe") {
    const auto ok = LerFases(ComAtaque(R"({ "estrategia": "PontosAttack", "projetil": "P",
        "pontos": { "posto": "quadro", "mirar": true } })"));
    CHECK(ok.problemas.empty());
    bool viu = false;
    for (const auto& f : ok.fases) {
        if (f.nome != "StateOne") continue;
        REQUIRE(f.ataques.size() == 1);
        REQUIRE(f.ataques[0].pontos.has_value());
        CHECK(f.ataques[0].pontos->posto == "quadro");
        CHECK(f.ataques[0].pontos->mirar);
        viu = true;
    }
    CHECK(viu);

    // Sem bloco, com bloco que nao e objeto, ou com um posto que ninguem ocupa.
    for (const char* resto : { "", R"(, "pontos": 3)", R"(, "pontos": { "posto": "nuvem" })",
                               R"(, "pontos": { "mirar": true })" }) {
        CAPTURE(resto);
        const auto r = LerFases(ComAtaque(std::string(R"({ "estrategia": "PontosAttack", "projetil": "P")")
                                          + resto + " }"));
        bool reclamou = false;
        for (const auto& p : r.problemas) if (p.find("pontos") != std::string::npos
                                              || p.find("PontosAttack") != std::string::npos) reclamou = true;
        CHECK(reclamou);
        for (const auto& f : r.fases) if (f.nome == "StateOne") CHECK(f.ataques.empty());
    }

    const auto fora = LerFases(ComAtaque(R"({ "estrategia": "WaveAttack", "projetil": "P",
                                              "pontos": { "posto": "quadro" } })"));
    bool avisou = false;
    for (const auto& p : fora.problemas) if (p.find("so a PontosAttack") != std::string::npos) avisou = true;
    CHECK(avisou);
}

// As etapas de uma fase (a cascata)

namespace {
    std::string FaseComEtapas(const std::string& etapas, const std::string& ataques) {
        return R"({
          "StateOne":   { "duracao": 10, "proximo": "StateFinal", "etapas": )" + etapas +
               R"(, "ataques": [ )" + ataques + R"( ] },
          "StateFinal": { "proximo": "", "ataques": [ { "estrategia": "WaveAttack", "projetil": "P" } ] }
        })";
    }
}

TEST_CASE("Fases: as etapas sao lidas em ordem, e cada ataque diz a sua") {
    const auto r = LerFases(FaseComEtapas(
        R"([ { "nome": "Requisitos", "duracao": 3 }, { "nome": "Projeto", "duracao": 4, "anuncio": "2 - Projeto" } ])",
        R"({ "estrategia": "WaveAttack", "projetil": "P", "etapa": "Projeto" },
           { "estrategia": "WaveAttack", "projetil": "P" })"));
    CHECK(r.problemas.empty());
    for (const auto& f : r.fases) {
        if (f.nome != "StateOne") continue;
        REQUIRE(f.etapas.size() == 2);
        CHECK(f.etapas[0].nome == "Requisitos");
        CHECK(f.etapas[0].anuncio.empty());
        CHECK(f.etapas[1].anuncio == "2 - Projeto");
        REQUIRE(f.ataques.size() == 2);
        CHECK(f.ataques[0].etapa == "Projeto");
        CHECK(f.ataques[1].etapa.empty());     // sem etapa: a fase inteira

        float de = -1.0f, ate = -1.0f;
        REQUIRE(JanelaDaEtapa(f.etapas, "Projeto", de, ate));
        CHECK(de == doctest::Approx(3.0f));
        CHECK(ate == doctest::Approx(7.0f));
        CHECK_FALSE(JanelaDaEtapa(f.etapas, "Testes", de, ate));
    }
}

TEST_CASE("Fases: a etapa de cada instante, e nenhuma depois da ultima") {
    std::vector<DescricaoDeEtapa> e(3);
    e[0].nome = "a"; e[0].duracao = 2.0f;
    e[1].nome = "b"; e[1].duracao = 2.0f;
    e[2].nome = "c"; e[2].duracao = 1.0f;
    CHECK(EtapaNoTempo(e, -1.0f) == 0);
    CHECK(EtapaNoTempo(e, 0.0f) == 0);
    CHECK(EtapaNoTempo(e, 1.99f) == 0);
    CHECK(EtapaNoTempo(e, 2.0f) == 1);     // a fronteira ja e da seguinte
    CHECK(EtapaNoTempo(e, 4.5f) == 2);
    CHECK(EtapaNoTempo(e, 5.0f) == -1);
    CHECK(EtapaNoTempo({}, 1.0f) == -1);
}

TEST_CASE("Fases: etapas mal escritas, ou que nao cabem na fase, sao recusadas") {
    const char* ruins[] = {
        R"([ { "nome": "a", "duracao": 0 } ])",
        R"([ { "nome": "a", "duracao": 3 }, { "nome": "a", "duracao": 3 } ])",
        R"([ { "duracao": 3 } ])",
        R"([ { "nome": "a", "duracao": 6 }, { "nome": "b", "duracao": 6 } ])",   // 12 s numa fase de 10
        R"([])",
        R"("a")"
    };
    for (const char* etapas : ruins) {
        CAPTURE(etapas);
        const auto r = LerFases(FaseComEtapas(etapas, R"({ "estrategia": "WaveAttack", "projetil": "P" })"));
        bool reclamou = false;
        for (const auto& p : r.problemas) if (p.find("etapas") != std::string::npos) reclamou = true;
        CHECK(reclamou);
    }
}

TEST_CASE("Fases: um ataque que pede uma etapa que a fase nao tem e descartado") {
    const auto r = LerFases(FaseComEtapas(
        R"([ { "nome": "Requisitos", "duracao": 3 } ])",
        R"({ "estrategia": "WaveAttack", "projetil": "P", "etapa": "Testes" },
           { "estrategia": "WaveAttack", "projetil": "P", "etapa": "Requisitos" })"));
    bool reclamou = false;
    for (const auto& p : r.problemas) if (p.find("Testes") != std::string::npos) reclamou = true;
    CHECK(reclamou);
    for (const auto& f : r.fases) {
        if (f.nome == "StateOne") CHECK(f.ataques.size() == 1);
    }
}

// O degrau do Dificil, por ataque.

namespace {
    std::string ComDificil(const std::string& bloco) {
        return std::string(R"({
  "StateOne": {
    "duracao": 17, "proximo": "StateFinal",
    "movimento": { "tipo": "RandomWander", "a": 4, "b": 100 },
    "ataques": [
      { "estrategia": "CircleSpreadAttack", "projetil": "Arduino",
        "projeteis": 36, "velocidade": 180, "cooldown": 2.4)") + bloco + R"( }
    ]
  },
  "StateFinal": {
    "duracao": 17, "proximo": "StateOne",
    "movimento": { "tipo": "GoToCenter" },
    "ataques": [
      { "estrategia": "CircleSpreadAttack", "projetil": "Arduino",
        "projeteis": 18, "velocidade": 120, "cooldown": 1.4 }
    ]
  }
})";
    }

    const DescricaoDeAtaque& PrimeiroDe(const FasesLidas& r, const char* fase) {
        for (const auto& f : r.fases) if (f.nome == fase) return f.ataques.at(0);
        throw std::runtime_error("sem fase");
    }
}

TEST_CASE("Fases: o bloco dificil troca so o que escreve, e so no Dificil") {
    const auto r = LerFases(ComDificil(R"(, "dificil": { "cooldown": 1.8, "velocidade": 230 })"));
    CHECK(r.problemas.empty());
    const DescricaoDeAtaque& a = PrimeiroDe(r, "StateOne");
    REQUIRE(a.dificil.has_value());

    const DescricaoDeAtaque normal = NoNivel(a, Dificuldade::Nivel::Normal);
    CHECK(normal.cooldown == doctest::Approx(2.4f));
    CHECK(*normal.velocidade == doctest::Approx(180.0f));
    CHECK(*normal.projeteis == 36);

    const DescricaoDeAtaque dificil = NoNivel(a, Dificuldade::Nivel::Dificil);
    CHECK(dificil.cooldown == doctest::Approx(1.8f));
    CHECK(*dificil.velocidade == doctest::Approx(230.0f));
    CHECK(*dificil.projeteis == 36);          // nao escrito: o do Normal

    // Sem bloco, o Dificil e o Normal.
    const DescricaoDeAtaque& sem = PrimeiroDe(r, "StateFinal");
    CHECK_FALSE(sem.dificil.has_value());
    CHECK(NoNivel(sem, Dificuldade::Nivel::Dificil).cooldown == doctest::Approx(1.4f));
}

TEST_CASE("Fases: o bloco dificil recusa o que ninguem le") {
    for (const char* ruim : {R"(, "dificil": { "cooldwn": 1.8 })",
                             R"(, "dificil": { "projeteis": 0 })",
                             R"(, "dificil": { "projeteis": 2.5 })",
                             R"(, "dificil": { "velocidade": -3 })",
                             R"(, "dificil": 3)"}) {
        CAPTURE(ruim);
        const auto r = LerFases(ComDificil(ruim));
        CHECK_FALSE(r.problemas.empty());
    }
    const auto ok = LerFases(ComDificil(R"(, "dificil": { "projeteis": 40 })"));
    CHECK(ok.problemas.empty());
    CHECK(*NoNivel(PrimeiroDe(ok, "StateOne"), Dificuldade::Nivel::Dificil).projeteis == 40);
}

// O degrau do Dificil, no movimento.

namespace {
    FasesLidas ComMovimento(const std::string& movimento) {
        return LerFases(R"({
  "StateOne": {
    "duracao": 20, "proximo": "",
    "movimento": )" + movimento + R"(,
    "ataques": [
      { "estrategia": "CircleSpreadAttack", "projetil": "Arduino",
        "projeteis": 8, "velocidade": 180, "cooldown": 2.4 }
    ]
  }
})");
    }
}

TEST_CASE("Fases: o bloco dificil do movimento troca so o que escreve, e so no Dificil") {
    const auto r = ComMovimento(R"({ "tipo": "Varredura", "a": 60, "b": 380, "dificil": { "a": 90 } })");
    CHECK(r.problemas.empty());
    REQUIRE(r.fases.size() == 1);
    const DescricaoDeMovimento& m = r.fases[0].movimento;

    const DescricaoDeMovimento normal = NoNivel(m, Dificuldade::Nivel::Normal);
    CHECK(normal.a == doctest::Approx(60.0f));
    CHECK(normal.b == doctest::Approx(380.0f));

    const DescricaoDeMovimento dificil = NoNivel(m, Dificuldade::Nivel::Dificil);
    CHECK(dificil.tipo == "Varredura");
    CHECK(dificil.a == doctest::Approx(90.0f));
    CHECK(dificil.b == doctest::Approx(380.0f));   // nao escrito: o do Normal

    // Sem bloco, o Dificil e o Normal.
    const auto sem = ComMovimento(R"({ "tipo": "Conduzido", "a": 500 })");
    REQUIRE(sem.fases.size() == 1);
    CHECK(NoNivel(sem.fases[0].movimento, Dificuldade::Nivel::Dificil).a == doctest::Approx(500.0f));
}

TEST_CASE("Fases: o bloco dificil do movimento avisa o que ninguem le, e fica com o resto") {
    for (const char* ruim : {R"({ "tipo": "Conduzido", "a": 400, "dificil": { "velocidade": 700 } })",
                             R"({ "tipo": "Conduzido", "a": 400, "dificil": { "a": "rapido" } })",
                             R"({ "tipo": "Conduzido", "a": 400, "dificil": 700 })"}) {
        CAPTURE(ruim);
        const auto r = ComMovimento(ruim);
        CHECK_FALSE(r.problemas.empty());
        REQUIRE(r.fases.size() == 1);
        CHECK(NoNivel(r.fases[0].movimento, Dificuldade::Nivel::Dificil).a == doctest::Approx(400.0f));
    }
    // A chave boa fica, mesmo ao lado de uma ruim.
    const auto meio = ComMovimento(R"({ "tipo": "Conduzido", "a": 400, "dificil": { "a": 700, "c": 1 } })");
    REQUIRE(meio.fases.size() == 1);
    CHECK(meio.problemas.size() == 1);
    CHECK(NoNivel(meio.fases[0].movimento, Dificuldade::Nivel::Dificil).a == doctest::Approx(700.0f));
}

// ---- O bloco "variar": o quanto cada disparo muda ----

TEST_CASE("Fases: o bloco variar e lido, e o que esta errado nele fica de fora com aviso") {
    const auto r = LerFases(R"({
 "StateOne": { "proximo": "StateFinal", "ataques": [
        { "estrategia": "WaveAttack", "projetil": "Capivara", "projeteis": 8, "angulo": 315,
          "cooldown": 2,
          "variar": { "giro": 12, "abertura": 0.15, "velocidade": 2, "projeteis": 1, "cadencia": 0.2,
                      "tamanho": 3 } } ] },
 "StateFinal": { "ataques": [
        { "estrategia": "CircleSpreadAttack", "projetil": "Capivara", "cooldown": 1 } ] }
    })");
    const auto& a = r.fases.at(1).ataques.at(0);   // StateOne
    REQUIRE(a.variar.has_value());
    CHECK(a.variar->giro == doctest::Approx(12.0f));
    CHECK(a.variar->abertura == doctest::Approx(0.15f));
    CHECK(a.variar->velocidade == 0.0f);      // 2 passa do teto: fica de fora
    CHECK(a.variar->projeteis == 1);
    CHECK(a.variar->cadencia == doctest::Approx(0.2f));
    bool velocidade = false, tamanho = false;
    for (const auto& p : r.problemas) {
        if (p.find("variar.velocidade") != std::string::npos) velocidade = true;
        if (p.find("variar.tamanho") != std::string::npos) tamanho = true;
    }
    CHECK(velocidade);
    CHECK(tamanho);

    // Sem o bloco, o ataque nao varia.
    CHECK_FALSE(r.fases.at(0).ataques.at(0).variar.has_value());
}

TEST_CASE("Fases: a cadencia so varia no ataque do relogio") {
    const auto r = LerFases(R"({
 "StateOne": { "proximo": "StateFinal", "ataques": [
        { "gatilho": "instrucao", "estrategia": "WaveAttack", "projetil": "Instrucao", "projeteis": 1,
          "variar": { "cadencia": 0.3, "velocidade": 0.1 } } ] },
 "StateFinal": { "ataques": [
        { "estrategia": "CircleSpreadAttack", "projetil": "Capivara", "cooldown": 1 } ] }
    })");
    const auto& a = r.fases.at(1).ataques.at(0);
    REQUIRE(a.variar.has_value());
    CHECK(a.variar->cadencia == 0.0f);
    CHECK(a.variar->velocidade == doctest::Approx(0.1f));
    bool avisou = false;
    for (const auto& p : r.problemas) if (p.find("variar.cadencia") != std::string::npos) avisou = true;
    CHECK(avisou);
}
