// O modo de plataforma do aluno: os quatro chaos, o pulo segurado e o pulo cortado.

#include "doctest.h"

#include <algorithm>
#include <array>

#include "../Source/Regras/Plataforma.h"

namespace {

    constexpr float kQuadro = 0.016f;
    constexpr float kAndar = 300.0f;
    constexpr std::array<Plataforma::Lado, 4> kLados = {Plataforma::Lado::Baixo, Plataforma::Lado::Esquerda,
                                                        Plataforma::Lado::Cima, Plataforma::Lado::Direita};

    /// O aluno parado no chao (o lado da gravidade passa pela origem) pula segurando o pulo por
    /// 'quadros' quadros: ate que altura ele chega, contada para longe do chao.
    float AlturaMaxima(const Plataforma::Lado l, const int quadros) {
        Plataforma::Estado e;
        const Vector2 g = Plataforma::Direcao(l);
        Vector2 p(0.0f, 0.0f);
        float maior = 0.0f;
        for (int q = 0; q < 400; ++q) {
            const Vector2 intencao = q < quadros ? g * -kAndar : Vector2::Zero;
            const bool encostou = Vector2::Dot(p, g) >= -0.5f;
            const Vector2 v = Plataforma::Passo(e, l, intencao, encostou, kQuadro, kAndar);
            p = p + v * kQuadro;
            // O chao: o centro nao passa dele (Player::PrenderNoCampo).
            if (Vector2::Dot(p, g) > 0.0f) p = p - g * Vector2::Dot(p, g);
            maior = std::max(maior, -Vector2::Dot(p, g));
        }
        return maior;
    }
}

TEST_CASE("Plataforma: a gravidade de cada lado e o eixo do chao, um perpendicular ao outro") {
    CHECK(Plataforma::Direcao(Plataforma::Lado::Baixo).y == doctest::Approx(1.0f));
    CHECK(Plataforma::Direcao(Plataforma::Lado::Esquerda).x == doctest::Approx(-1.0f));
    CHECK(Plataforma::Direcao(Plataforma::Lado::Cima).y == doctest::Approx(-1.0f));
    CHECK(Plataforma::Direcao(Plataforma::Lado::Direita).x == doctest::Approx(1.0f));
    // O de Baixo anda para a direita; os outros giram junto com a gravidade.
    CHECK(Plataforma::AoLongo(Plataforma::Lado::Baixo).x == doctest::Approx(1.0f));
    CHECK(Plataforma::AoLongo(Plataforma::Lado::Esquerda).y == doctest::Approx(1.0f));
    CHECK(Plataforma::AoLongo(Plataforma::Lado::Cima).x == doctest::Approx(-1.0f));
    CHECK(Plataforma::AoLongo(Plataforma::Lado::Direita).y == doctest::Approx(-1.0f));
    for (const Plataforma::Lado l : kLados) {
        CHECK(Vector2::Dot(Plataforma::Direcao(l), Plataforma::AoLongo(l)) == doctest::Approx(0.0f));
        CHECK(Plataforma::Direcao(l).Length() == doctest::Approx(1.0f));
    }
    CHECK(Plataforma::Giro(Plataforma::Lado::Baixo) == doctest::Approx(0.0f));
    CHECK(Plataforma::Giro(Plataforma::Lado::Esquerda) == doctest::Approx(90.0f));
    CHECK(Plataforma::Giro(Plataforma::Lado::Cima) == doctest::Approx(180.0f));
    CHECK(Plataforma::Giro(Plataforma::Lado::Direita) == doctest::Approx(270.0f));
}

TEST_CASE("Plataforma: no chao o aluno anda ao longo dele, e so empurrar para longe do chao pula") {
    for (const Plataforma::Lado l : kLados) {
        CAPTURE(static_cast<int>(l));
        const Vector2 g = Plataforma::Direcao(l);
        const Vector2 t = Plataforma::AoLongo(l);
        // Parado no chao: nada ao longo dele, e a queda segura no chao.
        Plataforma::Estado e;
        Vector2 v = Plataforma::Passo(e, l, Vector2::Zero, true, kQuadro, kAndar);
        CHECK(e.noChao);
        CHECK(Vector2::Dot(v, t) == doctest::Approx(0.0f));
        CHECK(Vector2::Dot(v, g) >= 0.0f);
        // Andando: a velocidade de sempre, ao longo do chao - em pixels de tela, o manche manda.
        v = Plataforma::Passo(e, l, t * kAndar, true, kQuadro, kAndar);
        CHECK(Vector2::Dot(v, t) == doctest::Approx(kAndar));
        CHECK(e.noChao);
        // Empurrar contra o chao nao pula.
        v = Plataforma::Passo(e, l, g * kAndar, true, kQuadro, kAndar);
        CHECK(e.noChao);
        CHECK(Vector2::Dot(v, g) >= 0.0f);
        // Empurrar para longe dele, sim.
        v = Plataforma::Passo(e, l, g * -kAndar, true, kQuadro, kAndar);
        CHECK_FALSE(e.noChao);
        CHECK(Vector2::Dot(v, g) < -500.0f);
    }
}

TEST_CASE("Plataforma: no ar nao se pula de novo, e o pulo sobe mais quanto mais tempo e segurado") {
    Plataforma::Estado e;
    const Plataforma::Lado l = Plataforma::Lado::Baixo;
    (void)Plataforma::Passo(e, l, Vector2(0.0f, -kAndar), true, kQuadro, kAndar);
    const float subindo = e.queda;
    // Fora do chao, empurrar para cima nao da outro impulso.
    (void)Plataforma::Passo(e, l, Vector2(0.0f, -kAndar), false, kQuadro, kAndar);
    CHECK(e.queda > subindo);

    // O toque e um pulinho; segurar ate o alto chega perto da conta, nos quatro lados.
    for (const Plataforma::Lado lado : kLados) {
        CAPTURE(static_cast<int>(lado));
        const float toque = AlturaMaxima(lado, 1);
        const float meio = AlturaMaxima(lado, 6);
        const float inteiro = AlturaMaxima(lado, 80);
        CHECK(toque < 40.0f);
        CHECK(toque < meio);
        CHECK(meio < inteiro);
        CHECK(inteiro == doctest::Approx(Plataforma::AlturaSegurando()).epsilon(0.08));
    }
    CHECK(Plataforma::AlturaSegurando() > 180.0f);
    CHECK(Plataforma::AlturaDoToque() < 20.0f);
}

TEST_CASE("Plataforma: a virada arremessa o aluno ao chao novo, e a queda tem limite") {
    Plataforma::Estado e;
    e.noChao = true;
    Plataforma::Virar(e);
    CHECK_FALSE(e.noChao);
    CHECK(e.queda == doctest::Approx(Plataforma::kBatida));
    // Caindo muito tempo, para no limite.
    for (int q = 0; q < 200; ++q) (void)Plataforma::Passo(e, Plataforma::Lado::Esquerda, Vector2::Zero, false, kQuadro, kAndar);
    CHECK(e.queda == doctest::Approx(Plataforma::kQuedaMaxima));
    // Ao encostar, a queda zera.
    (void)Plataforma::Passo(e, Plataforma::Lado::Esquerda, Vector2::Zero, true, kQuadro, kAndar);
    CHECK(e.noChao);
    CHECK(e.queda < 40.0f);
}
