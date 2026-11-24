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

TEST_CASE("Masmorra: o professor acende as tochas, uma de cada lado, antes do bau") {
    Variacao::Sorteio s(13u);
    const Vector2 professor(600.0f, 95.0f);
    const Degrau n = DegrauDo(Dificuldade::Nivel::Normal), d = DegrauDo(Dificuldade::Nivel::Dificil);
    for (int k = 0; k < 50; ++k) {
        const Vector2 bau = LugarDoBau(s, kCampo, Vector2(s.Real(100.0f, 1100.0f), 650.0f));
        const auto tochas = LugaresDasTochas(s, kTochas, kCampo, bau, professor);
        REQUIRE(tochas.size() == static_cast<size_t>(kTochas));
        // Uma em cada metade da sala, no alto (o tiro do aluno chega), longe do bau.
        CHECK(std::min(tochas[0].x, tochas[1].x) < kCampo.w * 0.5f);
        CHECK(std::max(tochas[0].x, tochas[1].x) > kCampo.w * 0.5f);
        for (const Vector2& t : tochas) {
            CHECK(t.y <= kCampo.h * 0.45f);
            CHECK((t - bau).Length() >= 150.0f);
        }
        // A mais perto primeiro.
        CHECK((tochas[0] - professor).Length() <= (tochas[1] - professor).Length() + 0.01f);
        // O caminho todo da tempo ao aluno - e o Dificil e mais curto.
        const float normal = Percurso(professor, tochas, bau, n.rapidezDoProfessor, n.acender);
        CHECK(normal > 6.0f);
        CHECK(Percurso(professor, tochas, bau, d.rapidezDoProfessor, d.acender) < normal);
    }
    // A proxima: a primeira apagada; todas acesas, o bau.
    CHECK(ProximaTocha({false, false}) == 0);
    CHECK(ProximaTocha({true, false}) == 1);
    CHECK(ProximaTocha({false, true}) == 0);       // a do aluno apagou a primeira: ele volta
    CHECK(ProximaTocha({true, true}) == -1);
    // O passo nao passa do destino.
    CHECK((Passo(Vector2(0.0f, 0.0f), Vector2(100.0f, 0.0f), 190.0f, 0.1f) - Vector2(19.0f, 0.0f)).Length() < 0.01f);
    CHECK((Passo(Vector2(0.0f, 0.0f), Vector2(10.0f, 0.0f), 190.0f, 0.1f) - Vector2(10.0f, 0.0f)).Length() < 0.01f);
    // O Dificil: professor mais rapido, mais gosmas, furia maior.
    CHECK(d.rapidezDoProfessor > n.rapidezDoProfessor);
    CHECK(d.acender < n.acender);
    CHECK(d.gosmas > n.gosmas);
    CHECK(d.bracos > n.bracos);
    CHECK(d.entreRajadas < n.entreRajadas);
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

TEST_CASE("Masmorra: a fadinha vai ao inimigo mais perto, e a mira cerca o alvo") {
    const std::vector<Vector2> alvos{Vector2(500.0f, 100.0f), Vector2(120.0f, 300.0f), Vector2(900.0f, 600.0f)};
    CHECK(MaisPerto(Vector2(100.0f, 400.0f), alvos) == 1);
    CHECK(MaisPerto(Vector2(950.0f, 650.0f), alvos) == 2);
    CHECK(MaisPerto(Vector2(0.0f, 0.0f), {}) == -1);
    // Ela paira em cima do alvo, perto dele.
    for (float t = 0.0f; t < 3.0f; t += 0.1f) {
        const Vector2 p = PousoDaFada(Vector2(300.0f, 300.0f), t);
        CHECK(p.y < 300.0f);
        CHECK((p - Vector2(300.0f, 300.0f)).Length() < kAcimaDoAlvo + 20.0f);
    }
    // A mira: quatro setas com a ponta para dentro, em volta do alvo.
    const Vector2 alvo(400.0f, 400.0f);
    const auto setas = Mira(alvo, 30.0f, 1.3f);
    REQUIRE(setas.size() == 4u);
    for (const Seta& s : setas) {
        CHECK((s.ponta - alvo).Length() < (s.esquerda - alvo).Length());
        CHECK((s.ponta - alvo).Length() < (s.direita - alvo).Length());
        CHECK((s.ponta - alvo).Length() == doctest::Approx(30.0f).epsilon(0.1));
    }
}
