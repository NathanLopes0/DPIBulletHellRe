//
// A sprint dos metodos ageis: a lei de Fitts, o minimo solavanco, o quadro e
// o caminho de cada historia ate a revisao.
//

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <set>
#include <vector>


#include "../Source/Attacks/Sprint.h"

namespace {
    float Dist(const Vector2 a, const Vector2 b) { return (a - b).Length(); }
}

TEST_CASE("Sprint: a lei de Fitts - mais longe demora mais, alvo mais largo demora menos") {
    const Sprint::Fitts f{0.35f, 0.18f};

    // D = 3L: log2(4) = 2 bits.
    CHECK(Sprint::TempoDeFitts(300.0f, 100.0f, f) == doctest::Approx(0.35f + 0.18f * 2.0f));
    // Distancia zero: so o 'a', o tempo de reagir.
    CHECK(Sprint::TempoDeFitts(0.0f, 100.0f, f) == doctest::Approx(0.35f));

    CHECK(Sprint::TempoDeFitts(400.0f, 100.0f, f) > Sprint::TempoDeFitts(200.0f, 100.0f, f));
    CHECK(Sprint::TempoDeFitts(300.0f, 800.0f, f) < Sprint::TempoDeFitts(300.0f, 40.0f, f));

    // Largura que nao e positiva vale um pixel, e nao divide por zero.
    CHECK(std::isfinite(Sprint::TempoDeFitts(300.0f, 0.0f, f)));
    CHECK(Sprint::TempoDeFitts(300.0f, 0.0f, f) == doctest::Approx(Sprint::TempoDeFitts(300.0f, 1.0f, f)));
}

TEST_CASE("Sprint: o minimo solavanco sai parado, passa no meio na metade e chega parado") {
    CHECK(Sprint::SemSolavanco(0.0f) == doctest::Approx(0.0f));
    CHECK(Sprint::SemSolavanco(0.5f) == doctest::Approx(0.5f));
    CHECK(Sprint::SemSolavanco(1.0f) == doctest::Approx(1.0f));
    CHECK(Sprint::SemSolavanco(-2.0f) == doctest::Approx(0.0f));
    CHECK(Sprint::SemSolavanco(3.0f) == doctest::Approx(1.0f));

    // Sai e chega DEVAGAR: no primeiro e no ultimo decimo anda menos que um
    // centesimo do caminho, e no decimo do meio anda mais de um oitavo.
    CHECK(Sprint::SemSolavanco(0.1f) < 0.01f);
    CHECK(1.0f - Sprint::SemSolavanco(0.9f) < 0.01f);
    CHECK(Sprint::SemSolavanco(0.55f) - Sprint::SemSolavanco(0.45f) > 0.125f);

    // E nunca volta.
    float antes = 0.0f;
    for (int k = 1; k <= 100; ++k) {
        const float agora = Sprint::SemSolavanco(static_cast<float>(k) / 100.0f);
        CHECK(agora >= antes);
        antes = agora;
    }
}

TEST_CASE("Sprint: as vagas ficam centradas no quadro, uma ao lado da outra") {
    Sprint::Quadro q;
    q.centroX = 600.0f; q.altura = 450.0f; q.espaco = 110.0f;

    CHECK(Sprint::Vaga(0, 1, q).x == doctest::Approx(600.0f));
    CHECK(Sprint::Vaga(0, 1, q).y == doctest::Approx(450.0f));

    CHECK(Sprint::Vaga(0, 2, q).x == doctest::Approx(545.0f));
    CHECK(Sprint::Vaga(1, 2, q).x == doctest::Approx(655.0f));

    // A sprint de oito: de 215 a 985, dentro da tela de 1200.
    CHECK(Sprint::Vaga(0, 8, q).x == doctest::Approx(215.0f));
    CHECK(Sprint::Vaga(7, 8, q).x == doctest::Approx(985.0f));
    for (int i = 0; i < 8; ++i) CHECK(Sprint::Vaga(i, 8, q).y == doctest::Approx(450.0f));
}

TEST_CASE("Sprint: o plano arrasta a pilha do professor ao meio do quadro, e a sprint cheia vai mais depressa") {
    Sprint::Quadro q;
    const Vector2 professor(600.0f, 150.0f);

    const auto uma = Sprint::Planejar(professor, 1, 2.7f, q);
    const auto oito = Sprint::Planejar(professor, 8, 2.7f, q);

    CHECK(uma.vagas.size() == 1);
    CHECK(oito.vagas.size() == 8);
    CHECK(uma.soltura.x == doctest::Approx(q.centroX));
    CHECK(uma.soltura.y == doctest::Approx(q.altura));

    // Fitts: o alvo da sprint de oito e o quadro inteiro.
    CHECK(oito.arraste < uma.arraste);
    CHECK(uma.arraste == doctest::Approx(Sprint::TempoDeFitts(q.altura - 150.0f, q.vaga)));

    CHECK(uma.espalha == doctest::Approx(Sprint::kEspalha));
    CHECK(uma.revisao == doctest::Approx(2.7f));
}

TEST_CASE("Sprint: a revisao nunca vem antes de as historias chegarem as vagas") {
    const auto p = Sprint::Planejar(Vector2(100.0f, 100.0f), 1, 0.1f);
    CHECK(p.revisao == doctest::Approx(p.arraste + p.espalha));
}

TEST_CASE("Sprint: a historia vai na pilha, se espalha, e espera na vaga ate a revisao") {
    const Vector2 professor(480.0f, 200.0f);
    const auto p = Sprint::Planejar(professor, 3, 2.7f);

    for (int i = 0; i < 3; ++i) {
        CAPTURE(i);
        // No disparo, na pilha em cima do professor.
        const Vector2 nasce = Sprint::PosicaoDaHistoria(p, i, 0.0f);
        CHECK(Dist(nasce, professor + Sprint::NaPilha(i)) < 0.01f);

        // Enquanto arrasta, a pilha vai com a ponta do cursor.
        const float meio = 0.5f * p.arraste;
        CHECK(Dist(Sprint::PosicaoDaHistoria(p, i, meio), Sprint::PontaDoCursor(p, meio) + Sprint::NaPilha(i)) < 0.01f);

        // Depois de espalhada, na vaga, e la fica.
        const Vector2 vaga = p.vagas[static_cast<size_t>(i)];
        CHECK(Dist(Sprint::PosicaoDaHistoria(p, i, p.arraste + p.espalha), vaga) < 0.01f);
        CHECK(Dist(Sprint::PosicaoDaHistoria(p, i, p.revisao - 0.01f), vaga) < 0.01f);
    }
}

TEST_CASE("Sprint: a historia nao salta - nem ao soltar, nem ao chegar na vaga") {
    const auto p = Sprint::Planejar(Vector2(900.0f, 120.0f), 5, 2.7f);

    // O maior passo em 1/60 s, do disparo a revisao. O cursor mais rapido do
    // minimo solavanco anda 1,875 vezes a media; 40 px num quadro ja seria um
    // salto que o olho ve.
    for (int i = 0; i < 5; ++i) {
        float maior = 0.0f;
        Vector2 antes = Sprint::PosicaoDaHistoria(p, i, 0.0f);
        for (float t = 1.0f / 60.0f; t < p.revisao; t += 1.0f / 60.0f) {
            const Vector2 agora = Sprint::PosicaoDaHistoria(p, i, t);
            maior = std::max(maior, Dist(agora, antes));
            antes = agora;
        }
        CAPTURE(i);
        CHECK(maior < 40.0f);
    }

    // E ao soltar, a pilha esta exatamente onde o cursor a largou.
    const float eps = 1.0e-4f;
    for (int i = 0; i < 5; ++i) {
        CHECK(Dist(Sprint::PosicaoDaHistoria(p, i, p.arraste - eps),
                   Sprint::PosicaoDaHistoria(p, i, p.arraste + eps)) < 0.5f);
    }
}

TEST_CASE("Sprint: o cursor arrasta, clica ao soltar, espera, clica antes da revisao e some nela") {
    const auto p = Sprint::Planejar(Vector2(600.0f, 150.0f), 2, 2.7f);

    CHECK(Sprint::GestoDoCursor(p, 0.0f) == Sprint::Gesto::Arrastando);
    CHECK(Sprint::GestoDoCursor(p, p.arraste + 0.01f) == Sprint::Gesto::Soltando);
    CHECK(Sprint::GestoDoCursor(p, p.arraste + p.espalha + 0.01f) == Sprint::Gesto::Esperando);
    CHECK(Sprint::GestoDoCursor(p, p.revisao - Sprint::kAvisoDaRevisao + 0.01f) == Sprint::Gesto::Clicando);
    CHECK(Sprint::GestoDoCursor(p, p.revisao) == Sprint::Gesto::Acabou);

    CHECK(Dist(Sprint::PontaDoCursor(p, 0.0f), p.origem) < 0.01f);
    CHECK(Dist(Sprint::PontaDoCursor(p, p.arraste), p.soltura) < 0.01f);
    CHECK(Dist(Sprint::PontaDoCursor(p, p.revisao), p.soltura) < 0.01f);
}

TEST_CASE("Sprint: a ponta da seta e o pixel (6, 4) do desenho") {
    // O desenho tem 32 px e o centro dele e o (16, 16): a ponta fica 10 px a
    // esquerda e 12 acima do centro, na escala do projetil.
    const Vector2 c = Sprint::CentroDoCursor(Vector2(100.0f, 200.0f), 1.0f);
    CHECK(c.x == doctest::Approx(110.0f));
    CHECK(c.y == doctest::Approx(212.0f));
    const Vector2 c2 = Sprint::CentroDoCursor(Vector2(100.0f, 200.0f), 1.5f);
    CHECK(c2.x == doctest::Approx(115.0f));
    CHECK(c2.y == doctest::Approx(218.0f));
}

TEST_CASE("Sprint: o planning poker de cada luta embaralha as quatro primeiras cartas, e o 8 fecha a fase") {
    std::set<std::vector<int>> vistas;
    for (std::uint32_t semente = 0; semente < 300; ++semente) {
        Variacao::Sorteio s = Variacao::SorteioDa(semente, "StateTwo");
        const Sprint::Poker p = Sprint::Sortear(s);
        CHECK(p.back() == 8);
        std::vector<int> cartas(p.begin(), p.end());
        std::sort(cartas.begin(), cartas.end());
        CHECK(cartas == std::vector<int>(Sprint::kBaralho.begin(), Sprint::kBaralho.end()));
        vistas.insert(std::vector<int>(p.begin(), p.end() - 1));
    }
    // AS 24 ORDENS das quatro primeiras aparecem.
    CHECK(vistas.size() == 24u);
}

TEST_CASE("Sprint: o placar da sprint diz os pontos, e o do baralho em ordem e o do arquivo") {
    // OS TEXTOS DE marialucia/fases.json, que o placar mostrava antes do sorteio.
    CHECK(Sprint::Anuncio(1, Sprint::kBaralho[0]) == "Sprint 1 \xc2\xb7 1 ponto");
    CHECK(Sprint::Anuncio(2, Sprint::kBaralho[1]) == "Sprint 2 \xc2\xb7 2 pontos");
    CHECK(Sprint::Anuncio(3, Sprint::kBaralho[2]) == "Sprint 3 \xc2\xb7 3 pontos");
    CHECK(Sprint::Anuncio(4, Sprint::kBaralho[3]) == "Sprint 4 \xc2\xb7 5 pontos");
    CHECK(Sprint::Anuncio(5, Sprint::kBaralho[4]) == "Sprint 5 \xc2\xb7 8 pontos");
    CHECK(Sprint::Anuncio(3, 1) == "Sprint 3 \xc2\xb7 1 ponto");
}
