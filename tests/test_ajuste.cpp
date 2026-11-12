//
// O ajuste automatico de dificuldade (Regras/Ajuste.h) e a conta de reprovacoes seguidas.
//

#include "doctest.h"
#include "../Source/Regras/Ajuste.h"
#include "../Source/Regras/ContaDaNota.h"
#include "../Source/Aluno/Progresso.h"

using namespace Ajuste;

TEST_CASE("ajuste: um degrau a cada duas reprovacoes seguidas, ate dois") {
    const auto normal = Dificuldade::Nivel::Normal;
    CHECK(NivelPara(0, false, normal) == 0);
    CHECK(NivelPara(1, false, normal) == 0);
    CHECK(NivelPara(2, false, normal) == 1);
    CHECK(NivelPara(3, false, normal) == 1);
    CHECK(NivelPara(4, false, normal) == 2);
    CHECK(NivelPara(9, false, normal) == 2);
}

TEST_CASE("ajuste: nada para quem ja passou, nem no Dificil") {
    CHECK(NivelPara(6, true, Dificuldade::Nivel::Normal) == 0);
    CHECK(NivelPara(6, false, Dificuldade::Nivel::Dificil) == 0);
}

TEST_CASE("ajuste: na dupla vale o menor") {
    CHECK(DaDupla({2, 1}) == 1);
    CHECK(DaDupla({0, 2}) == 0);
    CHECK(DaDupla({}) == 0);
}

TEST_CASE("ajuste: o que cada degrau muda") {
    CHECK(Do(0).velocidade == doctest::Approx(1.0f));
    CHECK(Do(0).perda == doctest::Approx(1.0f));
    CHECK(Do(1).velocidade == doctest::Approx(0.94f));
    CHECK(Do(1).perda == doctest::Approx(0.85f));
    CHECK(Do(2).velocidade == doctest::Approx(0.88f));
    CHECK(Do(2).perda == doctest::Approx(0.70f));
    CHECK(Do(7).nivel == 2);
    CHECK(Do(-1).nivel == 0);
}

TEST_CASE("ajuste: a conta da nota perde menos com o alivio") {
    Nota::Conta sem(80.0f);
    Nota::Conta com(80.0f);
    com.DefinirAlivio(Do(2).perda);
    const float perdaSem = 80.0f - (sem.Descer(), sem.Valor());
    const float perdaCom = 80.0f - (com.Descer(), com.Valor());
    CHECK(perdaCom == doctest::Approx(perdaSem * 0.7f));
}

TEST_CASE("ajuste: as reprovacoes seguidas zeram ao aprovar") {
    Progresso p;
    p.RegistrarNota(0, 30.0f, Dificuldade::Nivel::Normal);
    p.RegistrarNota(0, 45.0f, Dificuldade::Nivel::Dificil);   // conta em qualquer nivel
    CHECK(p.ReprovacoesSeguidas(0) == 2);
    p.RegistrarNota(0, 70.0f, Dificuldade::Nivel::Normal);
    CHECK(p.ReprovacoesSeguidas(0) == 0);
    p.RegistrarNota(0, 20.0f, Dificuldade::Nivel::Normal);
    CHECK(p.ReprovacoesSeguidas(0) == 1);
    CHECK(p.ReprovacoesSeguidas(5) == 0);
}
