//
// O Programa de Intercambio: para onde cada aluno vai, e quando.
//

#include "doctest.h"

#include "../Source/Gameplay/Intercambio.h"

#include <vector>

// A PARTE QUE DA PARA TESTAR DE UMA MECANICA QUE MEXE NOS JOGADORES.
//
// Battle move os alunos, desenha os marcadores e troca o placar, e nada disso
// entra na suite. O que entra sao as duas contas: para onde cada um vai, e em
// que quadro.

TEST_CASE("Intercambio: em dupla, um vai para o lugar do outro") {
    const auto d = Intercambio::Destinos({Vector2(300, 682), Vector2(900, 650)}, 1200.0f);
    REQUIRE(d.size() == 2);
    CHECK(d[0].x == doctest::Approx(900.0f));
    CHECK(d[0].y == doctest::Approx(650.0f));
    CHECK(d[1].x == doctest::Approx(300.0f));
    CHECK(d[1].y == doctest::Approx(682.0f));
}

TEST_CASE("Intercambio: sozinho, o aluno vai para o espelho e fica na mesma altura") {

    // O ESPELHO E DA LARGURA, e nao do centro do aluno: x vira largura - x. Um
    // aluno no meio fica no meio - o intercambio nao o tira do lugar, e isso e
    // o certo: o lugar "do outro aluno" e o mesmo.
    const auto d = Intercambio::Destinos({Vector2(200, 682)}, 1200.0f);
    REQUIRE(d.size() == 1);
    CHECK(d[0].x == doctest::Approx(1000.0f));
    CHECK(d[0].y == doctest::Approx(682.0f));

    const auto meio = Intercambio::Destinos({Vector2(600, 500)}, 1200.0f);
    CHECK(meio[0].x == doctest::Approx(600.0f));
}

TEST_CASE("Intercambio: sem aluno, nao ha destino") {
    CHECK(Intercambio::Destinos({}, 1200.0f).empty());
}

TEST_CASE("Intercambio: a troca acontece a cada periodo, e o aviso acende antes") {

    Intercambio::Relogio r;
    r.aCada = 8.0f;
    r.aviso = 1.5f;

    // Ate 6,5s, nada.
    CHECK_FALSE(Intercambio::Avancar(r, 6.4f));
    CHECK_FALSE(Intercambio::Avisando(r));

    // De 6,5s ate a troca, o aviso fica aceso.
    CHECK_FALSE(Intercambio::Avancar(r, 0.2f));
    CHECK(Intercambio::Avisando(r));

    // Aos 8s, a troca - e o aviso apaga no mesmo quadro. Este quadro passa da
    // conta em 0,1s, de proposito.
    CHECK(Intercambio::Avancar(r, 1.5f));
    CHECK_FALSE(Intercambio::Avisando(r));

    // A SEGUNDA TROCA E 8s DEPOIS DA PRIMEIRA, e nao 8s depois do quadro em que
    // ela foi notada: os 0,1s que sobraram contam. Sem isso ela sairia aos
    // 16,1s, e cada troca atrasaria um pouco mais que a anterior.
    CHECK(r.t == doctest::Approx(0.1f));
    CHECK_FALSE(Intercambio::Avancar(r, 7.85f));
    CHECK(Intercambio::Avancar(r, 0.05f));
}

TEST_CASE("Intercambio: um relogio invalido nunca troca nem avisa") {

    // Aviso do tamanho do periodo seria um aviso aceso para sempre; periodo zero,
    // uma troca a cada quadro.
    for (const auto& r0 : {Intercambio::Relogio{8.0f, 8.0f, 0.0f},
                           Intercambio::Relogio{0.0f, 0.0f, 0.0f},
                           Intercambio::Relogio{8.0f, -1.0f, 0.0f}}) {
        auto r = r0;
        CAPTURE(r.aCada);
        CAPTURE(r.aviso);
        CHECK_FALSE(Intercambio::Valido(r));
        CHECK_FALSE(Intercambio::Avancar(r, 100.0f));
        CHECK_FALSE(Intercambio::Avisando(r));
    }
}
