//
// O mural dos formados: quem passou no TCC, e quando.
//

#include "doctest.h"

#include "../Source/Aluno/Mural.h"

namespace {

constexpr int kTcc = 10;

Exportacao::FichaDeAluno Ficha(const std::string& matricula, const float nota, const std::string& quando) {
    Exportacao::FichaDeAluno f;
    f.matricula = matricula;
    f.progresso.RegistrarNota(kTcc, nota, Dificuldade::Nivel::Normal, quando);
    return f;
}

} // namespace

TEST_CASE("Mural: so entra quem passou no TCC, do mais recente ao mais antigo") {
    const std::vector<Exportacao::FichaDeAluno> fichas = {
        Ficha("111", 75.0f, "2026-09-01 10:00:00"),
        Ficha("222", 50.0f, "2026-10-05 10:00:00"),   // reprovou: fora
        Ficha("333", 62.0f, "2026-10-02 09:30:00"),
        Ficha("444", 90.0f, "2026-10-02 18:00:00"),
    };
    const auto mural = Mural::Recentes(fichas, kTcc, 10);
    REQUIRE(mural.size() == 3);
    CHECK(mural[0].matricula == "444");
    CHECK(mural[1].matricula == "333");
    CHECK(mural[2].matricula == "111");
    CHECK(mural[0].quando == "2026-10-02 18:00:00");
}

TEST_CASE("Mural: a data e a da primeira aprovacao, e nao a da ultima prova") {
    Exportacao::FichaDeAluno f = Ficha("555", 61.0f, "2026-09-01 10:00:00");
    f.progresso.RegistrarNota(kTcc, 95.0f, Dificuldade::Nivel::Normal, "2026-11-01 10:00:00");
    f.progresso.RegistrarNota(kTcc, 30.0f, Dificuldade::Nivel::Normal, "2026-11-02 10:00:00");
    const auto mural = Mural::Recentes({f}, kTcc, 10);
    REQUIRE(mural.size() == 1);
    CHECK(mural[0].quando == "2026-09-01 10:00:00");
}

TEST_CASE("Mural: quem se formou sem data (save antigo) vem no fim, e o mural tem um teto") {
    std::vector<Exportacao::FichaDeAluno> fichas = {
        Ficha("900", 80.0f, ""),
        Ficha("100", 80.0f, ""),
        Ficha("500", 80.0f, "2026-01-01 00:00:00"),
    };
    const auto mural = Mural::Recentes(fichas, kTcc, 10);
    REQUIRE(mural.size() == 3);
    CHECK(mural[0].matricula == "500");
    CHECK(mural[1].matricula == "100");
    CHECK(mural[2].matricula == "900");

    CHECK(Mural::Recentes(fichas, kTcc, 2).size() == 2);
    CHECK(Mural::Recentes(fichas, -1, 10).empty());
}

TEST_CASE("Mural: a data aparece no jeito de casa") {
    CHECK(Mural::Data("2026-10-02 14:30:00") == "02/10/2026");
    CHECK(Mural::Data("2026-10-02") == "02/10/2026");
    CHECK(Mural::Data("").empty());
    CHECK(Mural::Data("ontem").empty());
    CHECK(Mural::Data("2026/10/02 14:30").empty());
}
