// Progresso academico: o recorde por materia e o ponto de retomada.
// Camada pura: sem Game, sem SDL.

#include "doctest.h"
#include "../Source/Progresso.h"

// ---------------------------------------------------------------------------
// Materia nunca jogada
// ---------------------------------------------------------------------------

TEST_CASE("Progresso: materia nunca jogada tem recorde zero") {
    const Progresso p;
    CHECK(p.MelhorNota(99) == doctest::Approx(0.f));
    CHECK_FALSE(p.Aprovado(99));
}

TEST_CASE("Progresso: consultar NAO cria registro") {
    // O getter antigo usava operator[], que inseria uma entrada zero a cada
    // leitura: mutacao dentro de uma consulta, e o mapa crescia com materias
    // que ninguem jogou.
    Progresso p;
    for (int m = 0; m < 50; ++m) {
        (void)p.MelhorNota(m);
        (void)p.Aprovado(m);
        (void)p.NotaDeRetomada(m);
    }
    CHECK(p.QuantasRegistradas() == 0);
}

TEST_CASE("Progresso: materia nunca jogada retoma da nota inicial") {
    const Progresso p;
    CHECK(p.NotaDeRetomada(1) == doctest::Approx(Progresso::kNotaInicial));
}

// ---------------------------------------------------------------------------
// O recorde so sobe
// ---------------------------------------------------------------------------

TEST_CASE("Progresso: o recorde sobe quando a nota e melhor") {
    Progresso p;
    p.RegistrarNota(1, 55.f);
    CHECK(p.MelhorNota(1) == doctest::Approx(55.f));
    p.RegistrarNota(1, 80.f);
    CHECK(p.MelhorNota(1) == doctest::Approx(80.f));
}

TEST_CASE("Progresso: uma tentativa ruim NAO apaga o recorde") {
    // Era o bug visivel na tela de selecao: a variavel se chamava highScore mas
    // mostrava a ULTIMA nota, entao jogar mal baixava o recorde exibido.
    Progresso p;
    p.RegistrarNota(1, 85.f);
    p.RegistrarNota(1, 45.f);
    CHECK(p.MelhorNota(1) == doctest::Approx(85.f));
}

TEST_CASE("Progresso: uma tentativa ruim NAO re-tranca uma materia aprovada") {
    // Consequencia da anterior, e a mais grave: como as regras de desbloqueio
    // leem o recorde, apagar o recorde re-trancava a coluna seguinte.
    Progresso p;
    p.RegistrarNota(1, 70.f);
    CHECK(p.Aprovado(1));
    p.RegistrarNota(1, 41.f);
    CHECK(p.Aprovado(1));
}

TEST_CASE("Progresso: a retomada acompanha a ULTIMA nota, nao o recorde") {
    // As duas coisas que o mapa antigo confundia, agora separadas: o recorde e
    // um registro, a retomada e onde a proxima batalha comeca.
    Progresso p;
    p.RegistrarNota(1, 85.f);
    p.RegistrarNota(1, 50.f);
    CHECK(p.MelhorNota(1) == doctest::Approx(85.f));
    CHECK(p.NotaDeRetomada(1) == doctest::Approx(50.f));
}

TEST_CASE("Progresso: a retomada nunca cai abaixo da nota inicial") {
    Progresso p;
    p.RegistrarNota(1, 10.f);
    CHECK(p.NotaDeRetomada(1) == doctest::Approx(Progresso::kNotaInicial));
    CHECK(p.MelhorNota(1) == doctest::Approx(10.f));   // o recorde guarda o real
}

// ---------------------------------------------------------------------------
// Aprovacao
// ---------------------------------------------------------------------------

TEST_CASE("Progresso: a nota de aprovacao e inclusiva") {
    Progresso p;
    p.RegistrarNota(1, Progresso::kNotaDeAprovacao - 0.1f);
    CHECK_FALSE(p.Aprovado(1));
    p.RegistrarNota(1, Progresso::kNotaDeAprovacao);
    CHECK(p.Aprovado(1));
}

TEST_CASE("Progresso: conta as aprovadas de uma lista") {
    Progresso p;
    p.RegistrarNota(1, 70.f);
    p.RegistrarNota(2, 45.f);
    p.RegistrarNota(3, 61.f);
    CHECK(p.QuantasAprovadas({1, 2, 3}) == 2);
    CHECK(p.QuantasAprovadas({2}) == 0);
    CHECK(p.QuantasAprovadas({}) == 0);
    CHECK(p.QuantasAprovadas({1, 99}) == 1);   // 99 nunca jogada
}

// ---------------------------------------------------------------------------
// As regras de desbloqueio, expressas sobre o progresso
// ---------------------------------------------------------------------------

TEST_CASE("Desbloqueio: a coluna 3 exige duas aprovacoes na coluna 2") {
    const std::vector<int> col2 = {2, 3, 4, 5};
    Progresso p;

    CHECK(p.QuantasAprovadas(col2) < 2);
    p.RegistrarNota(2, 70.f);
    CHECK(p.QuantasAprovadas(col2) < 2);
    p.RegistrarNota(3, 65.f);
    CHECK(p.QuantasAprovadas(col2) >= 2);

    // E continua desbloqueada depois de uma tentativa ruim numa delas.
    p.RegistrarNota(2, 42.f);
    CHECK(p.QuantasAprovadas(col2) >= 2);
}
