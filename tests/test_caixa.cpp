//
// A caixa do HTML: os pontos do contorno e a velocidade que a faz crescer.
//

#include "doctest.h"

#include "../Source/Attacks/Caixa.h"

#include <cmath>
#include <vector>

// A PARTE QUE DA PARA TESTAR DE UMA ESTRATEGIA QUE CRIA PROJETEIS.
//
// CaixaAttack pede projeteis ao pool e nao entra na suite. O que entra e a
// geometria, e e nela que estao os erros que desmanchariam a fase: pontos fora
// do contorno, um lado com buraco, e uma caixa que vira losango enquanto cresce.

namespace {
    /// O ponto esta em cima do contorno do retangulo?
    bool NoContorno(const Vector2& p, const float mw, const float mh) {
        const float e = 0.01f;
        const bool emX = std::fabs(std::fabs(p.x) - mw) < e && std::fabs(p.y) <= mh + e;
        const bool emY = std::fabs(std::fabs(p.y) - mh) < e && std::fabs(p.x) <= mw + e;
        return emX || emY;
    }
}

TEST_CASE("Caixa: todos os pontos ficam no contorno, e o primeiro e o canto de cima a esquerda") {
    const auto p = Caixa::PontosDoContorno(32, 112.0f, 70.0f);
    REQUIRE(p.size() == 32);
    for (const auto& q : p) {
        CAPTURE(q.x);
        CAPTURE(q.y);
        CHECK(NoContorno(q, 112.0f, 70.0f));
    }
    CHECK(p[0].x == doctest::Approx(-112.0f));
    CHECK(p[0].y == doctest::Approx(-70.0f));
}

TEST_CASE("Caixa: os pontos sao igualmente espacados pelo contorno, sem buraco num lado") {

    // PELO COMPRIMENTO, e nao pelo angulo. Contando quantos caem em cada lado:
    // num retangulo 224 x 140 com 32 pontos o passo e 22,75, e os lados
    // compridos levam mais pontos que os curtos na proporcao do comprimento.
    const auto p = Caixa::PontosDoContorno(32, 112.0f, 70.0f);
    int topo = 0, base = 0, esquerda = 0, direita = 0;
    for (const auto& q : p) {
        if (std::fabs(q.y + 70.0f) < 0.01f && q.x < 112.0f - 0.01f) ++topo;
        else if (std::fabs(q.x - 112.0f) < 0.01f && q.y < 70.0f - 0.01f) ++direita;
        else if (std::fabs(q.y - 70.0f) < 0.01f && q.x > -112.0f + 0.01f) ++base;
        else ++esquerda;
    }
    CHECK(topo == 10);
    CHECK(direita == 6);
    CHECK(base == 10);
    CHECK(esquerda == 6);
}

TEST_CASE("Caixa: a velocidade faz a caixa crescer sem deformar") {

    // DEPOIS DE t SEGUNDOS, todo ponto tem de estar no contorno de uma caixa
    // MAIOR e SEMELHANTE: mesma proporcao, meia-altura h0 + v*t.
    const float h0 = 70.0f, w0 = 112.0f, v = 120.0f, t = 2.5f;
    const float h = h0 + v * t;
    const float w = w0 * (h / h0);
    for (const auto& q : Caixa::PontosDoContorno(28, w0, h0)) {
        const Vector2 depois = q + Caixa::Velocidade(q, h0, v) * t;
        CAPTURE(depois.x);
        CAPTURE(depois.y);
        CHECK(NoContorno(depois, w, h));
    }

    // O meio do lado de cima anda exatamente a 'v'.
    const Vector2 meioDoTopo(0.0f, -h0);
    CHECK(Caixa::Velocidade(meioDoTopo, h0, v).Length() == doctest::Approx(v));
}

TEST_CASE("Caixa: sem ponto pedido, ou com um lado nulo, nao ha caixa") {
    CHECK(Caixa::PontosDoContorno(0, 112.0f, 70.0f).empty());
    CHECK(Caixa::PontosDoContorno(10, 0.0f, 70.0f).empty());
    CHECK(Caixa::PontosDoContorno(10, 112.0f, -1.0f).empty());
    CHECK(Caixa::Velocidade(Vector2(10.0f, 10.0f), 0.0f, 120.0f).Length() == doctest::Approx(0.0f));
}
