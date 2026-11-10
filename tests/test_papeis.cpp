// Os papeis da dupla: em quem o professor mira, e a cobertura do parceiro.

#include "doctest.h"

#include "../Source/Regras/Cobertura.h"
#include "../Source/Regras/Papeis.h"

TEST_CASE("Papeis: sozinho o alvo e sempre o mesmo, e na dupla os dois revezam enquanto incomodam igual") {
    Papeis::Incomodo m;
    CHECK(m.Alvo(0, 1) == 0u);
    CHECK(m.Alvo(5, 1) == 0u);
    CHECK(m.Foco(1) == -1);

    // Ninguem acertou nada: o revezamento de sempre.
    CHECK(m.Alvo(0, 2) == 0u);
    CHECK(m.Alvo(1, 2) == 1u);

    // Os dois no mesmo ritmo, um acerto a cada 0,12 s: continuam revezando.
    for (int i = 0; i < 100; ++i) {
        m.Acertou(0);
        m.Acertou(1);
        m.Avancar(0.12f);
    }
    CHECK(m.Foco(2) == -1);
    CHECK(m.Alvo(0, 2) == 0u);
    CHECK(m.Alvo(1, 2) == 1u);

    // E um pouco mais rapido que o outro nao basta para prender a mira.
    Papeis::Incomodo n;
    for (int i = 0; i < 100; ++i) {
        n.Acertou(0);
        if (i % 8 != 0) n.Acertou(1);
        n.Avancar(0.12f);
    }
    CHECK(n.Foco(2) == -1);
}

TEST_CASE("Papeis: quem ataca e o outro nao, chama a mira; parou, ela solta") {
    Papeis::Incomodo m;
    for (int i = 0; i < 30; ++i) {
        m.Acertou(1);
        m.Avancar(0.12f);
    }
    CHECK(m.Foco(2) == 1);
    CHECK(m.Alvo(0, 2) == 1u);
    CHECK(m.Alvo(1, 2) == 1u);

    // O incomodo esquece pela metade a cada meia-vida.
    const float antes = m.De(1);
    m.Avancar(Papeis::kMeiaVida);
    CHECK(m.De(1) == doctest::Approx(antes / 2.0f));

    // Sem acertar mais nada, ele esquece de vez, e a mira volta a revezar.
    m.Avancar(8.0f * Papeis::kMeiaVida);
    CHECK(m.Foco(2) == -1);
    CHECK(m.Alvo(0, 2) == 0u);

    // Um acerto so nao prende a mira de ninguem.
    Papeis::Incomodo n;
    n.Acertou(0);
    CHECK(n.Foco(2) == -1);
    n.Zerar();
    CHECK(n.De(0) == 0.0f);

    // Fora da dupla nao ha a quem incomodar.
    n.Acertou(7);
    CHECK(n.De(7) == 0.0f);
}

TEST_CASE("Cobertura: o projetil que ia passar pelo parceiro, e nao o que ia para longe") {
    const Vector2 parceiro(500.0f, 600.0f);
    const float raio = 40.0f;

    // Descendo bem em cima dele: ia passar.
    CHECK(Cobertura::IaPassarPerto(Vector2(500.0f, 300.0f), Vector2(0.0f, 400.0f), parceiro, raio));
    // Descendo meio de lado, mas dentro do raio no caminho: ia passar.
    CHECK(Cobertura::IaPassarPerto(Vector2(530.0f, 300.0f), Vector2(0.0f, 400.0f), parceiro, raio));
    // Descendo longe dele: nao.
    CHECK_FALSE(Cobertura::IaPassarPerto(Vector2(700.0f, 300.0f), Vector2(0.0f, 400.0f), parceiro, raio));
    // Indo embora: nao.
    CHECK_FALSE(Cobertura::IaPassarPerto(Vector2(500.0f, 300.0f), Vector2(0.0f, -400.0f), parceiro, raio));
    // Na direcao certa, mas devagar demais para chegar dentro do horizonte: nao.
    CHECK_FALSE(Cobertura::IaPassarPerto(Vector2(500.0f, 300.0f), Vector2(0.0f, 100.0f), parceiro, raio));
    // Parado ja em cima dele: era perigo do mesmo jeito.
    CHECK(Cobertura::IaPassarPerto(Vector2(510.0f, 590.0f), Vector2(0.0f, 0.0f), parceiro, raio));
    CHECK_FALSE(Cobertura::IaPassarPerto(Vector2(800.0f, 590.0f), Vector2(0.0f, 0.0f), parceiro, raio));

    // A cobertura e da dupla: vale um acerto inteiro, o dobro do acerto repartido.
    CHECK(Cobertura::kGanho == doctest::Approx(Nota::kGanhoPorAcerto));
}

TEST_CASE("Cobertura: vai no aluno quem passaria na faixa do raspao dele, e so se algo o atinge") {
    using Cobertura::Aluno;
    const Aluno aluno{Vector2(500.0f, 600.0f), 6.0f, true};
    const Vector2 descendo(0.0f, 400.0f);
    // A faixa: o colisor dele, o do projetil e o alcance do raspao.
    const float faixa = 6.0f + 8.0f + Raspao::kAlcance;
    CHECK(Cobertura::VaiNele(Vector2(500.0f + faixa - 1.0f, 300.0f), descendo, 8.0f, aluno));
    CHECK_FALSE(Cobertura::VaiNele(Vector2(500.0f + faixa + 1.0f, 300.0f), descendo, 8.0f, aluno));
    // O escudo e a piscada: nada o atinge, e ninguem precisa cobri-lo.
    const Aluno protegido{aluno.centro, aluno.raio, false};
    CHECK_FALSE(Cobertura::VaiNele(Vector2(500.0f, 300.0f), descendo, 8.0f, protegido));
}

TEST_CASE("Cobertura: o projetil vai num aluno so - o que ele pega primeiro - e sozinho nao vai em ninguem") {
    using Cobertura::Aluno;
    const Vector2 descendo(0.0f, 400.0f);
    const std::vector<Aluno> dupla = {{Vector2(300.0f, 600.0f), 6.0f, true},
                                      {Vector2(700.0f, 600.0f), 6.0f, true}};
    CHECK(Cobertura::AlunoNoCaminho(Vector2(300.0f, 300.0f), descendo, 8.0f, dupla) == 0);
    CHECK(Cobertura::AlunoNoCaminho(Vector2(700.0f, 300.0f), descendo, 8.0f, dupla) == 1);
    CHECK(Cobertura::AlunoNoCaminho(Vector2(500.0f, 300.0f), descendo, 8.0f, dupla) == -1);
    // Na diagonal: sai de cima do primeiro, mas e no segundo que ele vai.
    CHECK(Cobertura::AlunoNoCaminho(Vector2(300.0f, 300.0f), Vector2(400.0f, 300.0f), 8.0f, dupla) == 1);

    // Um na frente do outro, na mesma coluna: o de cima e o primeiro a ser pego.
    const std::vector<Aluno> emFila = {{Vector2(500.0f, 650.0f), 6.0f, true},
                                       {Vector2(500.0f, 560.0f), 6.0f, true}};
    CHECK(Cobertura::AlunoNoCaminho(Vector2(500.0f, 300.0f), descendo, 8.0f, emFila) == 1);
    // O de cima protegido: o projetil segue para o de baixo.
    const std::vector<Aluno> deEscudo = {emFila[0], {emFila[1].centro, emFila[1].raio, false}};
    CHECK(Cobertura::AlunoNoCaminho(Vector2(500.0f, 300.0f), descendo, 8.0f, deEscudo) == 0);

    // Sozinho, nao ha quem cubra.
    CHECK(Cobertura::AlunoNoCaminho(Vector2(300.0f, 300.0f), descendo, 8.0f, {dupla[0]}) == -1);
    CHECK(Cobertura::AlunoNoCaminho(Vector2(300.0f, 300.0f), descendo, 8.0f, {}) == -1);
}

// ---- A mira inteira: o incomodo, o revezamento e a vez dada ----

TEST_CASE("Mira: sozinho o alvo e sempre o mesmo, e na dupla sem incomodo os disparos revezam") {
    Papeis::Mira m;
    CHECK(m.Alvo(1) == 0u);
    m.Revezar(1);
    CHECK(m.Alvo(1) == 0u);
    CHECK(m.Foco(1) == -1);

    Papeis::Mira dupla;
    CHECK(dupla.Alvo(2) == 0u);
    dupla.Revezar(2);
    CHECK(dupla.Alvo(2) == 1u);
    dupla.Revezar(2);
    CHECK(dupla.Alvo(2) == 0u);
}

TEST_CASE("Mira: quem incomoda prende a mira, e a vez dada vale um disparo so por cima dela") {
    Papeis::Mira m;
    for (int i = 0; i < 10; ++i) m.Acertou(0);
    CHECK(m.Foco(2) == 0);
    CHECK(m.Alvo(2) == 0u);

    m.DarAVez(1, 2);
    CHECK(m.Alvo(2) == 1u);
    // O disparo saiu: a mira volta para quem incomoda.
    m.Revezar(2);
    CHECK(m.Alvo(2) == 0u);

    // Uma vez fora da lista e ignorada.
    m.DarAVez(5, 2);
    CHECK(m.Alvo(2) == 0u);
}

TEST_CASE("Mira: o incomodo esquece, e a mira solta") {
    Papeis::Mira m;
    for (int i = 0; i < 10; ++i) m.Acertou(1);
    CHECK(m.Foco(2) == 1);
    m.Avancar(30.0f);
    CHECK(m.Foco(2) == -1);
}

TEST_CASE("Mira: sem os papeis (Experimento.h), a mira nunca prende e os disparos so revezam") {
    Papeis::Mira m(false);
    for (int i = 0; i < 20; ++i) m.Acertou(0);
    CHECK(m.Foco(2) == -1);
    CHECK(m.Alvo(2) == 0u);
    m.Revezar(2);
    CHECK(m.Alvo(2) == 1u);
}
