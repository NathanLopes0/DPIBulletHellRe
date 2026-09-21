// Testes da camada pura de resolucao de referencial.
// Nao sobe SDL, nao cria Actor: compila e roda em segundos.

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "../Source/Attacks/PathAim.h"

namespace {
    /// Angulo do referencial, em graus, para assercoes legiveis.
    float AnguloEmGraus(const Referencial& r) {
        return Math::ToDegrees(Math::Atan2(r.sin, r.cos));
    }

    ContextoDeResolucao ContextoBase() {
        ContextoDeResolucao c;
        c.posicaoProjetil    = Vector2(100.f, 100.f);
        c.velocidadeProjetil = Vector2(0.f, 200.f);   // descendo
        c.posicaoJogador     = Vector2(300.f, 100.f); // a direita, mesma altura
        c.velocidadeJogador  = Vector2::Zero;
        c.temJogador         = true;
        return c;
    }
}

TEST_CASE("RF1: a origem e sempre a posicao do projetil") {
    auto c = ContextoBase();
    for (auto modo : {Mira::AlinharComVelocidade, Mira::MirarNoJogador,
                      Mira::MirarPrevendo, Mira::AnguloFixo}) {
        const auto r = ResolverReferencial(c, Mira(modo, 0.5f));
        CHECK(r.origem.x == doctest::Approx(100.f));
        CHECK(r.origem.y == doctest::Approx(100.f));
    }
}

TEST_CASE("RF1: AlinharComVelocidade aponta na direcao do voo") {
    auto c = ContextoBase();                       // velocidade (0, 200) = 90 graus
    const auto r = ResolverReferencial(c, Mira(Mira::AlinharComVelocidade));
    CHECK(AnguloEmGraus(r) == doctest::Approx(90.f));
}

TEST_CASE("RF1: MirarNoJogador ignora a velocidade e aponta para o jogador") {
    auto c = ContextoBase();                       // jogador a direita = 0 graus
    const auto r = ResolverReferencial(c, Mira(Mira::MirarNoJogador));
    CHECK(AnguloEmGraus(r) == doctest::Approx(0.f));
}

TEST_CASE("RF1: MirarPrevendo antecipa pela velocidade do jogador") {
    auto c = ContextoBase();
    c.velocidadeJogador = Vector2(0.f, 200.f);     // jogador descendo

    const auto semAntecipacao = ResolverReferencial(c, Mira(Mira::MirarPrevendo, 0.0f));
    const auto comAntecipacao = ResolverReferencial(c, Mira(Mira::MirarPrevendo, 1.0f));

    // Antecipacao zero tem de ser identica a mirar na posicao atual.
    CHECK(AnguloEmGraus(semAntecipacao) == doctest::Approx(0.f));
    // Com 1s, o alvo desceu 200px: o angulo passa a ser 45 graus.
    CHECK(AnguloEmGraus(comAntecipacao) == doctest::Approx(45.f));
}

TEST_CASE("RF1: AnguloFixo nao depende do mundo") {
    auto c = ContextoBase();
    // Comparamos cos/sin em vez do angulo: em +-180 graus o atan2 alterna de
    // sinal por erro de ponto flutuante (sin(pi) e ~-8.7e-8, nao zero exato), e
    // -180 e o MESMO angulo que +180. Assercao sobre a direcao nao tem esse
    // problema de descontinuidade.
    const auto r = ResolverReferencial(c, Mira(Mira::AnguloFixo, Math::Pi));
    CHECK(r.cos == doctest::Approx(-1.f));
    CHECK(r.sin == doctest::Approx(0.f).epsilon(0.001));

    // Mesmo sem jogador e sem velocidade, o resultado e o mesmo.
    c.temJogador = false;
    c.velocidadeProjetil = Vector2::Zero;
    const auto r2 = ResolverReferencial(c, Mira(Mira::AnguloFixo, Math::Pi));
    CHECK(r2.cos == doctest::Approx(-1.f));
    CHECK(r2.sin == doctest::Approx(0.f).epsilon(0.001));
}

TEST_CASE("RNF3: sem jogador, cai para alinhar com a velocidade") {
    auto c = ContextoBase();
    c.temJogador = false;
    const auto r = ResolverReferencial(c, Mira(Mira::MirarNoJogador));
    CHECK(AnguloEmGraus(r) == doctest::Approx(90.f));   // direcao do voo
}

TEST_CASE("RNF3: jogador em cima do projetil cai para a velocidade") {
    auto c = ContextoBase();
    c.posicaoJogador = c.posicaoProjetil;              // distancia zero
    const auto r = ResolverReferencial(c, Mira(Mira::MirarNoJogador));
    CHECK(AnguloEmGraus(r) == doctest::Approx(90.f));
}

TEST_CASE("RNF3: sem jogador e sem velocidade, devolve identidade") {
    auto c = ContextoBase();
    c.temJogador = false;
    c.velocidadeProjetil = Vector2::Zero;
    const auto r = ResolverReferencial(c, Mira(Mira::MirarNoJogador));
    CHECK(r.cos == doctest::Approx(1.f));
    CHECK(r.sin == doctest::Approx(0.f));
}

TEST_CASE("RNF3: nenhum modo produz NaN em nenhuma combinacao degenerada") {
    ContextoDeResolucao c;   // tudo zero, sem jogador
    for (auto modo : {Mira::AlinharComVelocidade, Mira::MirarNoJogador,
                      Mira::MirarPrevendo, Mira::AnguloFixo}) {
        const auto r = ResolverReferencial(c, Mira(modo, 0.5f));
        CHECK(r.cos == r.cos);          // NaN != NaN
        CHECK(r.sin == r.sin);
        // O referencial tem de ser sempre unitario.
        CHECK(r.cos * r.cos + r.sin * r.sin == doctest::Approx(1.f));
    }
}

TEST_CASE("O referencial e sempre unitario, em todos os modos") {
    auto c = ContextoBase();
    for (auto modo : {Mira::AlinharComVelocidade, Mira::MirarNoJogador,
                      Mira::MirarPrevendo, Mira::AnguloFixo}) {
        const auto r = ResolverReferencial(c, Mira(modo, 0.7f));
        CHECK(r.cos * r.cos + r.sin * r.sin == doctest::Approx(1.f));
    }
}

// ---------------------------------------------------------------------------
// RF4: homing exprimivel como caminho
//
// O HomingBehavior aponta o projetil para o jogador UMA vez, no instante do
// atraso, e mantem o modulo da velocidade. Um PathBehavior com forma Reta e
// Mira(MirarNoJogador) resolve o referencial exatamente nessa direcao. O teste
// abaixo compara as duas contas no nivel geometrico.
// ---------------------------------------------------------------------------

TEST_CASE("RF4: Mira(MirarNoJogador) reproduz a direcao do HomingBehavior") {
    auto c = ContextoBase();
    c.posicaoProjetil = Vector2(400.f, 100.f);
    c.posicaoJogador  = Vector2(250.f, 500.f);

    // Como o HomingBehavior calcula: direcao normalizada ate o jogador.
    Vector2 comoHoming = c.posicaoJogador - c.posicaoProjetil;
    comoHoming.Normalize();

    // Como o caminho calcula.
    const auto r = ResolverReferencial(c, Mira(Mira::MirarNoJogador));

    CHECK(r.cos == doctest::Approx(comoHoming.x));
    CHECK(r.sin == doctest::Approx(comoHoming.y));
}

TEST_CASE("RF2: o referencial e resolvido UMA vez, nao acompanha o alvo") {
    auto c = ContextoBase();
    const auto antes = ResolverReferencial(c, Mira(Mira::MirarNoJogador));

    // O jogador se move DEPOIS da resolucao.
    c.posicaoJogador = Vector2(100.f, 900.f);
    const auto depois = ResolverReferencial(c, Mira(Mira::MirarNoJogador));

    // Duas resolucoes distintas dao resultados distintos - o que confirma que
    // a funcao e pura e que "resolver uma vez so" e responsabilidade de quem
    // chama (PathBehavior, via flag 'started'), nao da geometria.
    CHECK(antes.cos != doctest::Approx(depois.cos));
}
