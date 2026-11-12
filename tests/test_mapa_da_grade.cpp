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
