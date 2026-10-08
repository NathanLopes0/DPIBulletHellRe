// Leitura de fases de ataque a partir de texto JSON, e a validacao das
// transicoes. Camada pura: nao abre arquivo, nao escreve log, nao sobe SDL.

#include "doctest.h"
#include "../Source/Attacks/FasesDeAtaque.h"
#include "../Source/Tabela.h"

// Um conjunto minimo que FECHA, para servir de base aos casos que mexem em uma
// coisa de cada vez.
static const char* kConjuntoValido = R"({
  "ricardo": {
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
  }
})";

// ---------------------------------------------------------------------------
// Leitura
// ---------------------------------------------------------------------------

TEST_CASE("Fases: le um conjunto completo") {
    const auto r = LerFases(kConjuntoValido);
    CHECK(r.problemas.empty());
    REQUIRE(r.conjuntos.count("ricardo") == 1);
    REQUIRE(r.conjuntos.at("ricardo").size() == 2);

    // O mapa de fases e ordenado por nome, entao StateFinal vem antes de StateOne.
    const auto& fases = r.conjuntos.at("ricardo");
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
    // O ponto central do formato. Se um campo omitido virasse zero (ou um padrao
    // copiado a mao), a ponte sobrescreveria o padrao do AttackParams e o ataque
    // migrado se comportaria diferente do mesmo ataque em C++.
    // Ricardo nunca mexe em params->angle: o arquivo dele tambem nao pode.
    const auto r = LerFases(kConjuntoValido);
    REQUIRE(r.conjuntos.count("ricardo") == 1);
    const auto& fases = r.conjuntos.at("ricardo");
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
      "salles": { "StateOne": { "proximo": "StateFinal", "ataques": [
          { "estrategia": "AngledAttack", "projetil": "Capivara",
            "projeteis": 3, "velocidade": 340, "angulo": 40,
            "anguloCentral": 90, "intervalo": 0.02, "cooldown": 0.8 } ] },
                  "StateFinal": { "ataques": [
          { "estrategia": "AngledAttack", "projetil": "Capivara", "cooldown": 1 } ] } }
    })");
    CHECK(r.problemas.empty());
    const auto& a = r.conjuntos.at("salles").at(1).ataques[0];   // StateOne
    REQUIRE(a.angulo.has_value());
    CHECK(*a.angulo == doctest::Approx(40.f));
    REQUIRE(a.anguloCentral.has_value());
    CHECK(*a.anguloCentral == doctest::Approx(90.f));
    REQUIRE(a.intervalo.has_value());
    CHECK(*a.intervalo == doctest::Approx(0.02f));
}

TEST_CASE("Fases: regras por nome e regras em linha") {
    const auto r = LerFases(R"({
      "x": {
        "StateOne": { "proximo": "StateFinal", "ataques": [
          { "estrategia": "AngledAttack", "projetil": "P", "cooldown": 1,
            "regras": "julio_fase2" } ] },
        "StateFinal": { "ataques": [
          { "estrategia": "AngledAttack", "projetil": "P", "cooldown": 1,
            "regras": [ { "quando": "pares",
                          "motion": {"tipo":"Tracking","atraso":0.3,"forca":3.2,"duracao":2.4} } ] } ] }
      }
    })");
    CHECK(r.problemas.empty());
    const auto& fases = r.conjuntos.at("x");

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
      "x": {
        // 36 projeteis: o anel precisa ser denso para nao dar para atravessar.
        "StateOne":   { "proximo": "StateFinal", "ataques": [
          { "estrategia": "CircleSpreadAttack", "projetil": "P", "cooldown": 2.4 } ] },
        "StateFinal": { "ataques": [
          { "estrategia": "CircleSpreadAttack", "projetil": "P", "cooldown": 1.4 } ] }
      }
    })");
    CHECK(r.problemas.empty());
    CHECK(r.conjuntos.at("x").size() == 2);
}

// ---------------------------------------------------------------------------
// O que e recusado
// ---------------------------------------------------------------------------

TEST_CASE("Fases: estrategia inexistente e recusada") {
    std::vector<std::string> ps;
    const auto r = LerFases(R"({ "x": { "StateOne": { "ataques": [
        { "estrategia": "AtaqueQueNaoExiste", "projetil": "P", "cooldown": 1 } ] } } })");
    CHECK_FALSE(r.problemas.empty());
    // A fase perdeu o unico ataque, entao a fase inteira caiu.
    CHECK(r.conjuntos.at("x").empty());
}

TEST_CASE("Fases: ataque sem projetil e recusado") {
    // Sem o nome da fabrica nao ha de onde tirar os projeteis, e o ataque
    // dispararia nada em silencio.
    const auto r = LerFases(R"({ "x": { "StateOne": { "ataques": [
        { "estrategia": "AngledAttack", "cooldown": 1 } ] } } })");
    CHECK_FALSE(r.problemas.empty());
    CHECK(r.conjuntos.at("x").empty());
}

TEST_CASE("Fases: cooldown zero e recusado") {
    // Cooldown zero dispara uma rajada por quadro e enche o pool em menos de um
    // segundo.
    const auto r = LerFases(R"({ "x": { "StateOne": { "ataques": [
        { "estrategia": "AngledAttack", "projetil": "P", "cooldown": 0 } ] } } })");
    CHECK_FALSE(r.problemas.empty());
    CHECK(r.conjuntos.at("x").empty());
}

TEST_CASE("Fases: projeteis abaixo de 1 e recusado") {
    const auto r = LerFases(R"({ "x": { "StateOne": { "ataques": [
        { "estrategia": "AngledAttack", "projetil": "P", "projeteis": 0, "cooldown": 1 } ] } } })");
    CHECK_FALSE(r.problemas.empty());
    CHECK(r.conjuntos.at("x").empty());
}

TEST_CASE("Fases: nome de fase inventado e recusado") {
    // A FSM procura os estados por TEXTO. Um nome livre nunca seria alcancado, e
    // a fase existiria no arquivo sem nunca rodar.
    const auto r = LerFases(R"({ "x": { "FaseDoMeio": { "ataques": [
        { "estrategia": "AngledAttack", "projetil": "P", "cooldown": 1 } ] } } })");
    CHECK_FALSE(r.problemas.empty());
    CHECK(r.conjuntos.at("x").empty());
}

TEST_CASE("Fases: movimento inexistente cai para GoToCenter e avisa") {
    const auto r = LerFases(R"({ "x": { "StateOne": {
        "movimento": { "tipo": "VoarEmCirculos" },
        "ataques": [ { "estrategia": "AngledAttack", "projetil": "P", "cooldown": 1 } ] } } })");
    CHECK_FALSE(r.problemas.empty());
    // A fase sobrevive: so o movimento foi trocado, e o ataque continua valendo.
    REQUIRE(r.conjuntos.at("x").size() == 1);
    CHECK(r.conjuntos.at("x")[0].movimento.tipo == "GoToCenter");
}

TEST_CASE("Fases: um ataque ruim nao leva os bons embora") {
    const auto r = LerFases(R"({ "x": { "StateOne": { "ataques": [
        { "estrategia": "AngledAttack", "projetil": "P", "cooldown": 1 },
        { "estrategia": "NaoExiste",    "projetil": "P", "cooldown": 1 },
        { "estrategia": "WaveAttack",   "projetil": "P", "cooldown": 2 } ] } } })");
    CHECK(r.problemas.size() == 1);
    REQUIRE(r.conjuntos.at("x").size() == 1);
    CHECK(r.conjuntos.at("x")[0].ataques.size() == 2);
}

TEST_CASE("Fases: texto que nao e JSON vira problema, nao excecao") {
    const auto r = LerFases(R"({ "x": { "StateOne": )");
    CHECK_FALSE(r.problemas.empty());
    CHECK(r.conjuntos.empty());
}

// ---------------------------------------------------------------------------
// Os nomes que existem
// ---------------------------------------------------------------------------

TEST_CASE("Fases: as listas de nomes validos") {
    CHECK(EstrategiaExiste("AngledAttack"));
    CHECK(EstrategiaExiste("CircleSpreadAttack"));
    CHECK(EstrategiaExiste("WaveAttack"));
    CHECK(EstrategiaExiste("BaloonAttack"));
    CHECK(EstrategiaExiste("LaserAttack"));
    CHECK_FALSE(EstrategiaExiste("angledattack"));   // sensivel a caixa

    CHECK(MovimentoExiste("RandomWander"));
    CHECK(MovimentoExiste("HoverAbovePlayer"));
    CHECK(MovimentoExiste("GoToCenter"));
    CHECK_FALSE(MovimentoExiste("Parado"));

    CHECK(NomeDeFaseExiste("StateOne"));
    CHECK(NomeDeFaseExiste("StateFinal"));
    CHECK_FALSE(NomeDeFaseExiste("StateFive"));
}

// ---------------------------------------------------------------------------
// Transicoes
// ---------------------------------------------------------------------------

TEST_CASE("Transicoes: um conjunto que fecha nao tem problema") {
    const auto r = LerFases(kConjuntoValido);
    CHECK(ValidarTransicoes(r.conjuntos.at("ricardo"), "ricardo").empty());
}

TEST_CASE("Transicoes: proximo com erro de digitacao e apontado") {
    // A armadilha que o formato de arquivo multiplicaria: a FSM procura o estado
    // por texto, loga um erro e o chefe CONGELA, sem travar o jogo.
    std::vector<DescricaoDeFase> fases(2);
    fases[0].nome = "StateOne";   fases[0].proximo = "StateTow";   // trocado
    fases[1].nome = "StateFinal"; fases[1].proximo = "StateOne";
    const auto ps = ValidarTransicoes(fases, "x");
    REQUIRE(ps.size() == 1);
    CHECK(ps[0].find("StateTow") != std::string::npos);
}

TEST_CASE("Transicoes: a falta de StateFinal e apontada") {
    // O caso que custou depuracao: nota entre 40 e 59 ao fim da terceira fase
    // manda a FSM para StateFinal. Sem o estado, o chefe congela.
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

// ---------------------------------------------------------------------------
// Cooldown derivado do ritmo
// ---------------------------------------------------------------------------

TEST_CASE("Cooldown: derivado do total do ritmo da investida") {
    // O que o C++ fazia com kRitmoInvestida.DuracaoTotal()/repeticoes. Sem isto,
    // migrar transformaria o cooldown num numero solto e mexer no ritmo deixaria
    // varios cacadores na tela, sem erro nenhum aparecer.
    // (0.6 + 1.0) * 7 - 1.0 = 10.2;  10.2 / 7 = 1.457142...
    const auto r = LerFases(R"({ "julio": {
      "StateOne": { "proximo": "StateFinal", "ataques": [
        { "estrategia": "AngledAttack", "projetil": "Dados",
          "cooldown": { "doRitmo": "total", "dividirPor": 7 },
          "regras": [ { "investidaRepetida": {
              "ritmo": { "investida": 0.6, "pausa": 1.0, "repeticoes": 7 },
              "velocidadeNaInvestida": 900, "velocidadeNaPausa": 15 } } ] } ] },
      "StateFinal": { "ataques": [
        { "estrategia": "AngledAttack", "projetil": "Dados", "cooldown": 1 } ] } } })");
    CHECK(r.problemas.empty());
    const auto& fases = r.conjuntos.at("julio");
    const DescricaoDeAtaque* a = nullptr;
    for (const auto& f : fases) if (f.nome == "StateOne") a = &f.ataques[0];
    REQUIRE(a != nullptr);
    CHECK(a->cooldown == doctest::Approx(10.2f / 7.0f));
}

TEST_CASE("Cooldown: ciclo, e o campo mais") {
    // Ciclo() = investida + pausa = 1.6;  1.6 + 0.5 = 2.1
    const auto r = LerFases(R"({ "x": {
      "StateOne": { "proximo": "StateFinal", "ataques": [
        { "estrategia": "AngledAttack", "projetil": "P",
          "cooldown": { "doRitmo": "ciclo", "mais": 0.5 },
          "regras": [ { "investidaRepetida": {
              "ritmo": { "investida": 0.6, "pausa": 1.0, "repeticoes": 5 },
              "velocidadeNaInvestida": 800, "velocidadeNaPausa": 15 } } ] } ] },
      "StateFinal": { "ataques": [
        { "estrategia": "AngledAttack", "projetil": "P", "cooldown": 1 } ] } } })");
    CHECK(r.problemas.empty());
    for (const auto& f : r.conjuntos.at("x"))
        if (f.nome == "StateOne") CHECK(f.ataques[0].cooldown == doctest::Approx(2.1f));
}

TEST_CASE("Cooldown: pedir o ritmo sem ter investida em linha e recusado") {
    // Regras APONTADAS POR NOME vivem em regras.json. Derivar o cooldown de um
    // ritmo escrito em outro arquivo criaria uma dependencia invisivel: mexer em
    // regras.json mudaria a cadencia sem nenhum sinal em fases.json.
    const auto r = LerFases(R"({ "x": { "StateOne": { "ataques": [
        { "estrategia": "AngledAttack", "projetil": "P",
          "cooldown": { "doRitmo": "total" },
          "regras": "julio_fase2" } ] } } })");
    CHECK_FALSE(r.problemas.empty());
    CHECK(r.conjuntos.at("x").empty());
}

TEST_CASE("Cooldown: doRitmo com nome errado e recusado") {
    const auto r = LerFases(R"({ "x": { "StateOne": { "ataques": [
        { "estrategia": "AngledAttack", "projetil": "P",
          "cooldown": { "doRitmo": "duracao" },
          "regras": [ { "investidaRepetida": {
              "ritmo": { "investida": 0.6, "pausa": 1.0, "repeticoes": 5 },
              "velocidadeNaInvestida": 800, "velocidadeNaPausa": 15 } } ] } ] } } })");
    CHECK_FALSE(r.problemas.empty());
    CHECK(r.conjuntos.at("x").empty());
}

// ---------------------------------------------------------------------------
// BaloonAttack: o bloco "balao"
// ---------------------------------------------------------------------------

static std::string ComBalao(const char* balao) {
    return std::string(R"({ "andre": {
      "StateOne": { "proximo": "StateFinal", "ataques": [
        { "estrategia": "BaloonAttack", "projetil": "Baloes",
          "projeteis": 10, "velocidade": 500, "cooldown": 3,
          "balao": )") + balao + R"( } ] },
      "StateFinal": { "ataques": [
        { "estrategia": "CircleSpreadAttack", "projetil": "Baloes", "cooldown": 1 } ] } } })";
}

TEST_CASE("Balao: le o bloco completo") {
    const auto r = LerFases(ComBalao(R"({ "lado": "Right", "spawnAleatorio": true,
                                          "centradoNoJogador": true, "deslocamento": 600 })"));
    CHECK(r.problemas.empty());
    const DescricaoDeAtaque* a = nullptr;
    for (const auto& f : r.conjuntos.at("andre")) if (f.nome == "StateOne") a = &f.ataques[0];
    REQUIRE(a != nullptr);
    REQUIRE(a->balao.has_value());
    CHECK(a->balao->lado == "Right");
    CHECK(a->balao->spawnAleatorio.value_or(false));
    CHECK(a->balao->centradoNoJogador.value_or(false));
    CHECK(*a->balao->deslocamento == doctest::Approx(600.f));
    CHECK(a->balao->pontosDeSpawn.empty());
}

TEST_CASE("Balao: BaloonAttack SEM o bloco e recusada") {
    // A estrategia faz dynamic_cast para BaloonAttackParams e, se falhar, escreve
    // no log e nao dispara nada. Recusar na leitura troca um ataque invisivel por
    // uma frase no arquivo.
    const auto r = LerFases(R"({ "x": { "StateOne": { "ataques": [
        { "estrategia": "BaloonAttack", "projetil": "P", "cooldown": 1 } ] } } })");
    CHECK_FALSE(r.problemas.empty());
    CHECK(r.conjuntos.at("x").empty());
}

TEST_CASE("Balao: lado ausente ou invalido e recusado") {
    CHECK_FALSE(LerFases(ComBalao(R"({ "spawnAleatorio": true })")).problemas.empty());
    CHECK_FALSE(LerFases(ComBalao(R"({ "lado": "None",  "spawnAleatorio": true })")).problemas.empty());
    CHECK_FALSE(LerFases(ComBalao(R"({ "lado": "right", "spawnAleatorio": true })")).problemas.empty());
    CHECK(LadoDeBalaoExiste("Up"));
    CHECK_FALSE(LadoDeBalaoExiste("None"));
}

TEST_CASE("Balao: modo de posicoes exatas exige os pontos") {
    const auto semPontos = LerFases(ComBalao(R"({ "lado": "Down", "spawnAleatorio": false,
                                                  "centradoNoJogador": false })"));
    CHECK_FALSE(semPontos.problemas.empty());

    const auto ok = LerFases(ComBalao(R"({ "lado": "Down", "spawnAleatorio": false,
                                           "centradoNoJogador": false,
                                           "pontosDeSpawn": [[10,20],[30,40]] })"));
    CHECK(ok.problemas.empty());
    for (const auto& f : ok.conjuntos.at("andre"))
        if (f.nome == "StateOne") {
            REQUIRE(f.ataques[0].balao->pontosDeSpawn.size() == 2);
            CHECK(f.ataques[0].balao->pontosDeSpawn[1].x == doctest::Approx(30.f));
        }
}

TEST_CASE("Balao: posicoes exatas exigem centradoNoJogador falso EXPLICITO") {
    // centerOnPlayer nasce TRUE no BaloonAttackParams, e a BaloonAttack recusa a
    // combinacao com randomSpawn falso. Omitir o campo aceitaria um ataque que
    // nunca dispara.
    const auto r = LerFases(ComBalao(R"({ "lado": "Down", "spawnAleatorio": false,
                                          "pontosDeSpawn": [[10,20]] })"));
    CHECK_FALSE(r.problemas.empty());
}

TEST_CASE("Balao: o bloco numa estrategia que nao usa e avisado, nao fatal") {
    const auto r = LerFases(R"({ "x": {
      "StateOne":   { "proximo": "StateFinal", "ataques": [
        { "estrategia": "AngledAttack", "projetil": "P", "cooldown": 1,
          "balao": { "lado": "Up" } } ] },
      "StateFinal": { "ataques": [
        { "estrategia": "AngledAttack", "projetil": "P", "cooldown": 1 } ] } } })");
    CHECK(r.problemas.size() == 1);
    // A fase sobrevive: o bloco foi so ignorado.
    REQUIRE(r.conjuntos.at("x").size() == 2);
    for (const auto& f : r.conjuntos.at("x"))
        if (f.nome == "StateOne") CHECK_FALSE(f.ataques[0].balao.has_value());
}

// ---------------------------------------------------------------- consulta

namespace {
    /// Uma ConsultaAttack com o bloco "consulta" que o teste quiser.
    std::string ComConsulta(const std::string& bloco) {
        return R"({ "x": { "StateOne": { "ataques": [
            { "estrategia": "ConsultaAttack", "projetil": "P", "cooldown": 1,
              "consulta": )" + bloco + R"( } ] } } })";
    }
}

TEST_CASE("Consulta: um indice fora da tabela e recusado") {

    // O DEFEITO QUE ISTO FECHA. A fase final do Thiago teve quatro ataques
    // dizendo varrer as faixas 0, 2, 1 e 3 de uma tabela de UMA faixa. A
    // estrategia limita para a ultima, entao os quatro varriam a mesma coisa - a
    // tela inteira - e nada avisava. O arquivo descrevia uma trelica que o jogo
    // nunca desenhou.
    CHECK_FALSE(LerFases(ComConsulta(R"({ "eixo": "Linha",  "linhas": 1, "indice": 2 })")).problemas.empty());
    CHECK_FALSE(LerFases(ComConsulta(R"({ "eixo": "Coluna", "colunas": 1, "indice": 3 })")).problemas.empty());
    CHECK_FALSE(LerFases(ComConsulta(R"({ "eixo": "Linha",  "linhas": 4, "indice": 4 })")).problemas.empty());
}

TEST_CASE("Consulta: o indice e conferido contra o EIXO, nao contra o outro") {

    // Uma consulta por linha de indice 4 numa tabela de 4 linhas e 5 colunas e
    // invalida, mesmo havendo 5 colunas. Conferir contra o eixo errado deixaria
    // passar exatamente o caso que este teste existe para pegar.
    CHECK_FALSE(LerFases(ComConsulta(
        R"({ "eixo": "Linha", "linhas": 4, "colunas": 5, "indice": 4 })")).problemas.empty());

    CHECK(LerFases(ComConsulta(
        R"({ "eixo": "Coluna", "linhas": 4, "colunas": 5, "indice": 4 })")).problemas.empty());
}

TEST_CASE("Consulta: sem \"linhas\" o padrao da Tabela e que vale") {

    // Omitir o numero de faixas e legitimo - o padrao de Tabela::Forma vale.
    // A conferencia do indice tem de usar ESSE padrao, senao um indice valido
    // seria recusado ou um invalido passaria, conforme o arquivo fosse explicito.
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

    // Aviso negativo e a varredura disparando antes de aparecer: o jogador seria
    // atingido por um ataque que nunca teve como ler.
    CHECK_FALSE(LerFases(ComConsulta(R"({ "eixo": "Linha", "aviso": -0.5 })")).problemas.empty());
    CHECK(LerFases(ComConsulta(R"({ "eixo": "Linha", "aviso": 0 })")).problemas.empty());
}
