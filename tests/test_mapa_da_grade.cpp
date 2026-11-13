//
// O mapa da grade da selecao de fases (Curso/MapaDaGrade.h) e as optativas em materias.json.
//

#include "doctest.h"
#include "../Source/Curso/MapaDaGrade.h"
#include "../Source/Curso/Materias.h"

using namespace MapaDaGrade;

TEST_CASE("mapa: sem optativas, as colunas ficam onde sempre ficaram") {
    const auto c = Colunas(5, 5, 1200.0f);
    REQUIRE(c.size() == 5);
    // margem 100, botao 128: de 164 a 1036, passo 218.
    CHECK(c[0].x == doctest::Approx(164.0f));
    CHECK(c[1].x == doctest::Approx(382.0f));
    CHECK(c[4].x == doctest::Approx(1036.0f));
    for (const auto& col : c) CHECK(col.pagina == 0);
    CHECK(Paginas(c) == 1);
}

TEST_CASE("mapa: as optativas vao para a pagina seguinte, sem o lugar do cartao") {
    const auto c = Colunas(5, 10, 1200.0f);
    CHECK(PorPagina(5) == 4);
    CHECK(c[5].pagina == 1);
    CHECK(c[5].x == doctest::Approx(1200.0f + 382.0f));   // o lugar da coluna 1
    CHECK(c[8].x == doctest::Approx(1200.0f + 1036.0f));  // o da coluna 4
    CHECK(c[9].pagina == 2);
    CHECK(c[9].x == doctest::Approx(2400.0f + 382.0f));
    CHECK(Paginas(c) == 3);
}

TEST_CASE("mapa: a camera chega, e encosta no fim") {
    CHECK(DeslocamentoDa(2, 1200.0f) == doctest::Approx(-2400.0f));
    float x = 0.0f;
    for (int i = 0; i < 10; ++i) x = Aproximar(x, -1200.0f, 0.016f);
    CHECK(x < -600.0f);
    CHECK(x > -1200.0f);
    for (int i = 0; i < 200; ++i) x = Aproximar(x, -1200.0f, 0.016f);
    CHECK(x == -1200.0f);
}

TEST_CASE("materias: a optativa e lida, e conta separado") {
    const auto l = Materias::LerMaterias(R"({ "materias": [
        { "codigo": "A", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "B", "coluna": 1, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "OPT1", "coluna": 2, "optativa": true, "desbloqueio": { "tipo": "aprovadoEm", "materias": ["A"] } }
    ] })");
    CHECK(l.problemas.empty());
    REQUIRE(l.Quantas() == 3);
    CHECK(l.Por(2)->optativa);
    CHECK_FALSE(l.Por(0)->optativa);
    CHECK(l.QuantasColunasObrigatorias() == 2);
    CHECK(l.QuantasColunas() == 3);
    CHECK(l.QuantasObrigatorias() == 2);
}

TEST_CASE("materias: optativa no meio das obrigatorias e avisada") {
    const auto l = Materias::LerMaterias(R"({ "materias": [
        { "codigo": "A", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "OPT1", "coluna": 0, "optativa": true, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "B", "coluna": 1, "desbloqueio": { "tipo": "sempre" } }
    ] })");
    bool avisou = false;
    for (const auto& p : l.problemas) if (p.find("OPT1") != std::string::npos) avisou = true;
    CHECK(avisou);
}

TEST_CASE("materias: na dupla, a materia so abre se estiver aberta para os dois") {
    const auto l = Materias::LerMaterias(R"({ "materias": [
        { "codigo": "A", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "B", "coluna": 1, "desbloqueio": { "tipo": "aprovadoEm", "materias": ["A"] } },
        { "codigo": "C", "coluna": 2, "desbloqueio": { "tipo": "aprovadoEm", "materias": ["B"] } }
    ] })");
    Progresso adiantado;
    adiantado.RegistrarNota(0, 80.0f, Dificuldade::Nivel::Normal);
    adiantado.RegistrarNota(1, 75.0f, Dificuldade::Nivel::Normal);
    Progresso atrasado;
    atrasado.RegistrarNota(0, 70.0f, Dificuldade::Nivel::Normal);

    // Sozinho, cada um ve o seu mapa.
    CHECK(l.DesbloqueadaParaTodos(2, {&adiantado}));
    CHECK_FALSE(l.DesbloqueadaParaTodos(2, {&atrasado}));
    // Juntos, vale o de quem esta mais atras, na ordem que for.
    CHECK(l.DesbloqueadaParaTodos(0, {&adiantado, &atrasado}));
    CHECK(l.DesbloqueadaParaTodos(1, {&adiantado, &atrasado}));
    CHECK_FALSE(l.DesbloqueadaParaTodos(2, {&adiantado, &atrasado}));
    CHECK_FALSE(l.DesbloqueadaParaTodos(2, {&atrasado, &adiantado}));
    CHECK_FALSE(l.DesbloqueadaParaTodos(0, {}));
}

// As ligacoes que acendem. A coluna 0 tem os botoes 0 (y 100) e 1 (y 300), a 1 tem o 2 (y 200),
// e a 2 tem o 3 (y 200): o tronco 0 liga as colunas 0 e 1, em dois pedacos (100-200 e 200-300);
// o tronco 1 liga as colunas 1 e 2, e nao tem pedaco nenhum - os dois botoes estao na mesma altura.
namespace {
    const std::vector<Trecho> kTrechos = {
        {0, -1, 100, 200}, {0, 0, 100}, {0, 1, 300}, {0, 2, 200}, {0, -1, 200, 300},   // tronco 0
        {1, 2, 200}, {1, 3, 200},                                                     // tronco 1
    };
    std::vector<size_t> Quais(const std::vector<Aceso>& a) {
        std::vector<size_t> q;
        for (const Aceso& x : a) q.push_back(x.trecho);
        return q;
    }
}

TEST_CASE("mapa: a luz corre do botao que saiu ao que chegou, so pelo pedaco do tronco entre eles") {
    const auto a = Caminho(kTrechos, 0, 2, 0.1f);
    // o ramo do 0, o pedaco 100-200 e o ramo do 2; o pedaco 200-300 fica apagado
    CHECK(Quais(a) == std::vector<size_t>{0, 1, 3});
    CHECK(a[1].atraso == doctest::Approx(0.0f));    // o ramo de quem saiu, primeiro
    CHECK(a[0].atraso == doctest::Approx(0.1f));    // o pedaco: meio caminho, 0.5 + 0.5 passo
    CHECK(a[2].atraso == doctest::Approx(0.2f));    // o ramo de quem chegou, por ultimo
}

TEST_CASE("mapa: na mesma coluna a luz passa pelos pedacos entre os dois, em ordem") {
    const auto a = Caminho(kTrechos, 0, 1, 0.1f);
    CHECK(Quais(a) == std::vector<size_t>{0, 1, 2, 4});
    CHECK(a[0].atraso < a[3].atraso);   // 100-200 antes de 200-300: a luz desce
}

TEST_CASE("mapa: na mesma altura, so os dois ramos") {
    CHECK(Quais(Caminho(kTrechos, 2, 3, 0.1f)) == std::vector<size_t>{5, 6});
}

TEST_CASE("mapa: sem tronco comum, e ao abrir a tela, nada acende") {
    CHECK(Caminho(kTrechos, 0, 3, 0.1f).empty());    // 0 e 3 nao se tocam
    CHECK(Caminho(kTrechos, -1, 2, 0.1f).empty());   // o primeiro foco
    CHECK(Caminho(kTrechos, 2, 2, 0.1f).empty());
}

TEST_CASE("mapa: a luz e efemera - sobe, fica cheia um instante e some") {
    CHECK(Luz(-0.01f) == 0.0f);
    CHECK(Luz(kSubidaDaLuz / 2.0f) == doctest::Approx(0.5f));
    CHECK(Luz(kSubidaDaLuz + kLuzCheia / 2.0f) == 1.0f);
    const float fim = kSubidaDaLuz + kLuzCheia + kDescidaDaLuz;
    CHECK(Luz(fim - kDescidaDaLuz / 2.0f) == doctest::Approx(0.25f));
    CHECK(Luz(fim) == 0.0f);
    CHECK(Luz(fim + 5.0f) == 0.0f);
}

TEST_CASE("mapa: a luz tambem corre para tras, da coluna da direita para a da esquerda") {
    const auto a = Caminho(kTrechos, 2, 1, 0.1f);
    CHECK(Quais(a) == std::vector<size_t>{2, 3, 4});
    CHECK(a[1].atraso == doctest::Approx(0.0f));    // agora quem saiu e o 2
}
