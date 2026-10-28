//
// O Ponto Extra do co-op: o escudo do parceiro e o feixe ate ele.
//

#include "doctest.h"

#include <cmath>

#include "../Source/PontoExtra.h"

TEST_CASE("PontoExtra: o escudo pega o projetil que encosta no anel, e nao o de fora") {
    const Vector2 aluno(400.0f, 600.0f);
    const float r = 8.0f;
    CHECK(PontoExtra::NoEscudo(aluno, aluno, r));
    CHECK(PontoExtra::NoEscudo(aluno, aluno + Vector2(PontoExtra::kRaioDoEscudo + r - 0.5f, 0.0f), r));
    CHECK_FALSE(PontoExtra::NoEscudo(aluno, aluno + Vector2(PontoExtra::kRaioDoEscudo + r + 0.5f, 0.0f), r));
    // O RAIO DO PROJETIL CONTA: um maior e pego mais longe.
    const Vector2 longe = aluno + Vector2(0.0f, PontoExtra::kRaioDoEscudo + 20.0f);
    CHECK_FALSE(PontoExtra::NoEscudo(aluno, longe, 8.0f));
    CHECK(PontoExtra::NoEscudo(aluno, longe, 24.0f));
}

TEST_CASE("PontoExtra: o anel do escudo fecha em volta do aluno, com todos os pontos no raio") {
    const Vector2 c(100.0f, 200.0f);
    const auto anel = PontoExtra::Anel(c, 50.0f, 24);
    REQUIRE(anel.size() == 24u);
    for (size_t k = 0; k < anel.size(); ++k) {
        CHECK((anel[k].first - c).Length() == doctest::Approx(50.0f).epsilon(0.001));
        // CADA LADO COMECA ONDE O ANTERIOR ACABOU, e o ultimo fecha no primeiro.
        const auto& proximo = anel[(k + 1) % anel.size()];
        CHECK((anel[k].second - proximo.first).Length() < 0.01f);
    }
    CHECK(PontoExtra::Anel(c, 50.0f, 1).size() == 3u);
}

TEST_CASE("PontoExtra: o feixe sai de quem gastou, chega no parceiro e o rastro se apaga") {
    const Vector2 de(100.0f, 600.0f), ate(500.0f, 600.0f);

    const auto inicio = PontoExtra::FeixeEm(de, ate, 0.0f);
    CHECK(inicio.cabeca);
    CHECK((inicio.cabecaAte - de).Length() < 0.01f);

    // A CABECA ANDA SO PARA A FRENTE, e e um traco curto.
    float antes = -1.0f;
    for (float t = 0.02f; t < PontoExtra::kViagemDoFeixe; t += 0.05f) {
        const auto f = PontoExtra::FeixeEm(de, ate, t);
        REQUIRE(f.cabeca);
        CHECK(f.cabecaAte.x > antes);
        antes = f.cabecaAte.x;
        CHECK((f.cabecaAte - f.cabecaDe).Length() <= PontoExtra::kTamanhoDoFeixe * 400.0f + 0.01f);
    }

    // CHEGOU: a cabeca some no parceiro, e o rastro vai de aceso a apagado.
    const auto chegou = PontoExtra::FeixeEm(de, ate, PontoExtra::kViagemDoFeixe);
    CHECK_FALSE(chegou.cabeca);
    CHECK(chegou.rastro == doctest::Approx(1.0f));
    const auto meio = PontoExtra::FeixeEm(de, ate, PontoExtra::kViagemDoFeixe + PontoExtra::kRastroDoFeixe / 2.0f);
    CHECK(meio.rastro == doctest::Approx(0.5f));
    const float fim = PontoExtra::kViagemDoFeixe + PontoExtra::kRastroDoFeixe;
    CHECK_FALSE(PontoExtra::FeixeAceso(fim));
    CHECK(PontoExtra::FeixeEm(de, ate, fim).rastro == 0.0f);
    CHECK_FALSE(PontoExtra::FeixeEm(de, ate, fim).cabeca);
}
