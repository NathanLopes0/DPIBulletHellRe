// O raspao: o projetil que passa rente ao aluno sem acerta-lo.

#include "doctest.h"

#include "../Source/Regras/Acerto.h"
#include "../Source/Regras/Raspao.h"

TEST_CASE("Raspao: a faixa vai da borda do colisor do aluno ate o alcance, sem encostar") {
    const Vector2 aluno(100.0f, 100.0f);
    const float raioDoAluno = 6.4f;   // um decimo do desenho de 64 px
    const float raio = 10.0f;

    // Encostando no colisor e acerto, e nao raspao.
    CHECK_FALSE(Raspao::NaFaixa(aluno, raioDoAluno, aluno + Vector2(raio + raioDoAluno - 1.0f, 0.0f), raio));
    // Logo depois da borda, raspao - de qualquer lado.
    CHECK(Raspao::NaFaixa(aluno, raioDoAluno, aluno + Vector2(raio + raioDoAluno + 1.0f, 0.0f), raio));
    CHECK(Raspao::NaFaixa(aluno, raioDoAluno, aluno + Vector2(0.0f, -(raio + 12.0f)), raio));
    // Na ponta do alcance ainda vale; um pouco alem, nao.
    CHECK(Raspao::NaFaixa(aluno, raioDoAluno, aluno + Vector2(raio + Raspao::kAlcance, 0.0f), raio));
    CHECK_FALSE(Raspao::NaFaixa(aluno, raioDoAluno, aluno + Vector2(raio + Raspao::kAlcance + 1.0f, 0.0f), raio));
    // O projetil maior raspa de mais longe: a faixa e contada da borda dele.
    CHECK(Raspao::NaFaixa(aluno, raioDoAluno, aluno + Vector2(40.0f, 0.0f), 22.0f));
    CHECK_FALSE(Raspao::NaFaixa(aluno, raioDoAluno, aluno + Vector2(40.0f, 0.0f), 10.0f));

    // A faixa e uma faixa, e nao o desenho inteiro: o alcance passa do colisor e fica dentro do
    // desenho do aluno (32 px de raio).
    CHECK(Raspao::kAlcance > raioDoAluno);
    CHECK(Raspao::kAlcance < 32.0f);
}

TEST_CASE("Raspao: a faisca nasce na borda do aluno, do lado de onde o projetil passou") {
    const Vector2 aluno(0.0f, 0.0f);
    const Vector2 f = Raspao::OndeFaisca(aluno, Vector2(100.0f, 0.0f));
    CHECK(f.x == doctest::Approx(Raspao::kAlcance));
    CHECK(f.y == doctest::Approx(0.0f));
    const Vector2 g = Raspao::OndeFaisca(aluno, Vector2(0.0f, -50.0f));
    CHECK(g.y == doctest::Approx(-Raspao::kAlcance));
    // Em cima do aluno nao ha lado: fica nele.
    const Vector2 h = Raspao::OndeFaisca(aluno, aluno);
    CHECK(h.x == doctest::Approx(0.0f));
}

TEST_CASE("Raspao: vale um terco de um acerto, e um projetil raspa uma vez so") {
    CHECK(Raspao::kGanho == doctest::Approx(Nota::kGanhoPorAcerto / 3.0f));

    // A memoria do raspao e a dos acertos, com o prazo da vida de um projetil.
    Acerto::Memoria m(Raspao::kMemoria);
    int tiro = 0;
    m.Lembrar(&tiro, 1.0f);
    // O mesmo projetil, mais velho, nao raspa de novo - nem depois do prazo dos acertos.
    m.Avancar(Acerto::kMesmoCausador + 0.5f);
    CHECK(m.Lembra(&tiro, 1.0f + Acerto::kMesmoCausador + 0.5f));
    m.Avancar(10.0f);
    CHECK(m.Lembra(&tiro, 12.0f));
    // O pool recicla o endereco: um projetil novo no mesmo lugar raspa outra vez.
    CHECK_FALSE(m.Lembra(&tiro, 0.1f));
    // E depois da vida de um projetil, a memoria esquece.
    m.Avancar(Raspao::kMemoria);
    CHECK(m.Quantos() == 0u);

    // A dos acertos continua com o prazo dela.
    Acerto::Memoria acertos;
    acertos.Lembrar(&tiro, 1.0f);
    acertos.Avancar(Acerto::kMesmoCausador + 0.01f);
    CHECK(acertos.Quantos() == 0u);
}
