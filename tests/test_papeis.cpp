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
