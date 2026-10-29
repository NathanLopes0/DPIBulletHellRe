//
// A geracao de texto da fase 1 do Julio: o token sai da ponta da frase.
//

#include "doctest.h"

#include <cmath>
#include <cstdint>
#include <set>

#include "../Source/Attacks/Julio/Geracao.h"
#include "../Source/Gameplay/Variacao.h"

TEST_CASE("Geracao: a diferenca de rumo vai pelo menor arco") {
    CHECK(Geracao::Diferenca(10.0f, 30.0f) == doctest::Approx(20.0f));
    CHECK(Geracao::Diferenca(350.0f, 10.0f) == doctest::Approx(20.0f));
    CHECK(Geracao::Diferenca(10.0f, 350.0f) == doctest::Approx(-20.0f));
    CHECK(std::fabs(Geracao::Diferenca(0.0f, 180.0f)) == doctest::Approx(180.0f));
}

TEST_CASE("Geracao: o rumo mais provavel vira para o alvo, no maximo a virada") {
    CHECK(Geracao::RumoProvavel(90.0f, 95.0f) == doctest::Approx(95.0f));
    CHECK(Geracao::RumoProvavel(90.0f, 150.0f) == doctest::Approx(90.0f + Geracao::kVirada));
    CHECK(Geracao::RumoProvavel(90.0f, 20.0f) == doctest::Approx(90.0f - Geracao::kVirada));
    // PELO MENOR ARCO: de 350 para 10 vira para cima, e nao 340 graus para baixo.
    CHECK(Geracao::RumoProvavel(350.0f, 10.0f) == doctest::Approx(350.0f + Geracao::kVirada));
}

TEST_CASE("Geracao: fria, o token sai sempre no rumo mais provavel; quente, se espalha") {
    // FRIA: a cobra e uma curva lisa para o aluno.
    for (float u = 0.0f; u < 1.0f; u += 0.05f) {
        const auto p = Geracao::Proximo(90.0f, 150.0f, 0.0f, u);
        CHECK(p.Rumo() == doctest::Approx(Geracao::RumoProvavel(90.0f, 150.0f)));
    }
    // QUENTE: os rumos sorteados cobrem os candidatos.
    Variacao::Sorteio s(7u);
    std::set<int> escolhidos;
    for (int i = 0; i < 500; ++i) escolhidos.insert(Geracao::Proximo(90.0f, 90.0f, 1.6f, s.Real(0.0f, 1.0f)).escolhido);
    CHECK(escolhidos.size() == static_cast<size_t>(Geracao::kCandidatos));

    // A DISTRIBUICAO QUE O CAMPO DESENHA: os candidatos em volta do provavel,
    // e a chance de cada um somando 1.
    const auto p = Geracao::Proximo(90.0f, 90.0f, 0.8f, 0.5f);
    REQUIRE(p.rumos.size() == static_cast<size_t>(Geracao::kCandidatos));
    float soma = 0.0f;
    for (const float x : p.probabilidades) soma += x;
    CHECK(soma == doctest::Approx(1.0f));
    CHECK(p.rumos.front() == doctest::Approx(90.0f - Geracao::kAbertura / 2.0f));
    CHECK(p.rumos.back() == doctest::Approx(90.0f + Geracao::kAbertura / 2.0f));
}

TEST_CASE("Geracao: a frase tem os sete tokens ate 1,0, e a alucinacao passa do fim") {
    CHECK(Geracao::TokensDaFrase(0.2f) == Geracao::kTokensDaFrase);
    CHECK(Geracao::TokensDaFrase(1.0f) == Geracao::kTokensDaFrase);
    CHECK(Geracao::TokensDaFrase(1.6f) == Geracao::kTokensDaFrase + 3);
    CHECK(Geracao::TokensDaFrase(2.2f) == Geracao::kTokensDaFrase + 6);
    CHECK(Geracao::TokensDaFrase(1.4f) < Geracao::TokensDaFrase(1.8f));
}

TEST_CASE("Geracao: o degrau do Normal e o de sempre, e o Dificil gera mais frases") {
    CHECK(Geracao::DegrauDo(Dificuldade::Nivel::Normal).entreFrases == doctest::Approx(2.0f));
    CHECK(Geracao::DegrauDo(Dificuldade::Nivel::Dificil).entreFrases <
          Geracao::DegrauDo(Dificuldade::Nivel::Normal).entreFrases);
}
