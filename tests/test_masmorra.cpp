//
// A fase 3 do Lucas: o Elixir, numa sala de masmorra.
//

#include "doctest.h"

#include "../Source/Attacks/Masmorra.h"

using namespace Masmorra;

namespace {
    const Campo kCampo{0.0f, 0.0f, 1200.0f, 720.0f};
}

TEST_CASE("Masmorra: o bau nasce longe do aluno, e as gosmas longe do bau") {
    Variacao::Sorteio s(11u);
    for (int k = 0; k < 50; ++k) {
        const Vector2 aluno(s.Real(100.0f, 1100.0f), 650.0f);
        const Vector2 bau = LugarDoBau(s, kCampo, aluno);
        CHECK(bau.y >= kCampo.h * 0.55f - 0.01f);
        CHECK((bau - aluno).Length() >= kLongeDoAluno * 0.6f);
        const auto gosmas = LugaresDasGosmas(s, 4, kCampo, bau);
        CHECK(gosmas.size() == 4u);
        for (const Vector2& g : gosmas) CHECK((g - bau).Length() >= 100.0f);
    }
}

TEST_CASE("Masmorra: o professor anda pelas beiradas e chega ao bau na hora marcada") {
    const Vector2 de(600.0f, 105.0f), bau(200.0f, 520.0f);
    const float chegada = 7.0f;
    CHECK((CaminhoDoProfessor(de, bau, 0.0f, chegada, kCampo) - de).Length() < 30.0f);
    CHECK((CaminhoDoProfessor(de, bau, chegada, chegada, kCampo) - bau).Length() < 0.5f);
    CHECK((CaminhoDoProfessor(de, bau, 99.0f, chegada, kCampo) - bau).Length() < 0.5f);
    // Primeiro de lado (no alto), depois a descida: na metade do tempo ele ainda nao desceu muito.
    const Vector2 meio = CaminhoDoProfessor(de, bau, chegada * 0.4f, chegada, kCampo);
    CHECK(meio.y < 150.0f);
    // Sem saltos.
    Vector2 antes = de;
    for (float t = 0.0f; t <= chegada; t += 0.016f) {
        const Vector2 p = CaminhoDoProfessor(de, bau, t, chegada, kCampo);
        CHECK((p - antes).Length() < 20.0f);
        antes = p;
    }
    // O Dificil: chega antes, mais gosmas, furia maior.
    const Degrau n = DegrauDo(Dificuldade::Nivel::Normal), d = DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(d.chegada < n.chegada);
    CHECK(d.gosmas > n.gosmas);
    CHECK(d.bracos > n.bracos);
    CHECK(d.entreRajadas < n.entreRajadas);
    CHECK(n.chegada + n.furia < kCiclo + 0.01f);   // a furia acaba dentro do ciclo
    CHECK(d.chegada + d.furia < kCiclo + 0.01f);
}

TEST_CASE("Masmorra: pegar e a espiral da furia") {
    CHECK(Alcanca(Vector2(100.0f, 100.0f), Vector2(120.0f, 100.0f)));
    CHECK_FALSE(Alcanca(Vector2(100.0f, 100.0f), Vector2(200.0f, 100.0f)));
    const auto rumos = RumosDaEspiral(4, 0.0f, 90.0f);
    REQUIRE(rumos.size() == 4u);
    CHECK(rumos[1] - rumos[0] == doctest::Approx(90.0f));
    const auto depois = RumosDaEspiral(4, 1.0f, 90.0f);
    CHECK(depois[0] == doctest::Approx(90.0f));               // a espiral gira
}
