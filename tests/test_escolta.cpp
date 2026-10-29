//
// O orientador em campo: onde ele fica, e para onde atira.
//

#include "doctest.h"

#include <cmath>
#include <vector>

#include "../Source/Gameplay/Orientador/Escolta.h"

namespace {
    constexpr float kEsquerda = 20.0f;
    constexpr float kDireita = 1180.0f;

    float Angulo(const Vector2& v) { return std::atan2(v.y, v.x) * 180.0f / Math::Pi; }
}

TEST_CASE("Escolta: sozinho ele fica de um lado, e so troca quando a parede tira o espaco") {
    // Comeca a esquerda e fica la enquanto couber.
    CHECK(Escolta::Lado(600.0f, -1, kEsquerda, kDireita) == -1);
    CHECK(Escolta::Lado(kEsquerda + Escolta::kMargem + Escolta::kAoLado + 1.0f, -1, kEsquerda, kDireita) == -1);
    // Encostado na parede da esquerda, vai para a direita...
    CHECK(Escolta::Lado(kEsquerda + 40.0f, -1, kEsquerda, kDireita) == 1);
    // ...e continua la mesmo de volta ao meio: trocar a cada passo o faria
    // atravessar o aluno o tempo todo.
    CHECK(Escolta::Lado(600.0f, 1, kEsquerda, kDireita) == 1);
    CHECK(Escolta::Lado(kDireita - 40.0f, 1, kEsquerda, kDireita) == -1);
}

TEST_CASE("Escolta: o posto e ao lado e acima do aluno, ou no meio da dupla e mais acima") {
    const Vector2 aluno(600.0f, 650.0f);
    const Vector2 esq = Escolta::Posto({aluno}, -1, kEsquerda, kDireita);
    CHECK(esq.x == doctest::Approx(600.0f - Escolta::kAoLado));
    CHECK(esq.y == doctest::Approx(650.0f - Escolta::kAcima));
    CHECK(Escolta::Posto({aluno}, 1, kEsquerda, kDireita).x == doctest::Approx(600.0f + Escolta::kAoLado));

    const Vector2 dupla = Escolta::Posto({Vector2(300.0f, 650.0f), Vector2(900.0f, 610.0f)}, -1, kEsquerda, kDireita);
    CHECK(dupla.x == doctest::Approx(600.0f));
    CHECK(dupla.y == doctest::Approx(630.0f - Escolta::kAcimaDaDupla));

    // Nunca fora do campo, nem que o lado nao tenha sido trocado ainda.
    CHECK(Escolta::Posto({Vector2(kEsquerda, 650.0f)}, -1, kEsquerda, kDireita).x
          == doctest::Approx(kEsquerda + Escolta::kMargem));

    // Sem ninguem em campo, um lugar que existe: o meio, no alto.
    CHECK(Escolta::Posto({}, -1, kEsquerda, kDireita).x == doctest::Approx(600.0f));
}

TEST_CASE("Escolta: ele chega ao posto com atraso, sem passar do ponto") {
    const Vector2 de(0.0f, 0.0f), para(100.0f, -50.0f);
    CHECK(Escolta::Seguir(de, para, 0.0f).x == doctest::Approx(0.0f));

    Vector2 p = de;
    for (int i = 0; i < 6; ++i) p = Escolta::Seguir(p, para, 1.0f / 60.0f);
    CHECK(p.x > 0.0f);
    CHECK(p.x < 100.0f);                      // um decimo de segundo nao basta

    for (int i = 0; i < 120; ++i) p = Escolta::Seguir(p, para, 1.0f / 60.0f);
    CHECK(p.x == doctest::Approx(100.0f).epsilon(0.01));
    CHECK(p.x <= 100.0f);                     // nao passa do ponto
    CHECK(p.y == doctest::Approx(-50.0f).epsilon(0.01));
}
