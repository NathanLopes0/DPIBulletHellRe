//
// A festa da matricula: as fichas que pagam, o pulo do losango, o anel e as faiscas.
//

#include "doctest.h"

#include <cmath>

#include "../Source/Curso/FestaDaMatricula.h"

using namespace FestaDaMatricula;

TEST_CASE("Festa da matricula: uma ficha a cada 5 h do preco, entre 3 e 8") {
    CHECK(Fichas(20) == 4);
    CHECK(Fichas(30) == 6);
    CHECK(Fichas(22) == 5);    // a sobra tambem paga uma
    CHECK(Fichas(5) == kMenosFichas);
    CHECK(Fichas(0) == kMenosFichas);
    CHECK(Fichas(200) == kMaisFichas);
}

TEST_CASE("Festa da matricula: as fichas saltam em arco, uma atras da outra, e chegam ao losango") {
    const Vector2 de(700.0f, 650.0f), para(400.0f, 300.0f);
    // O arco comeca e termina nos dois pontos, e passa acima do mais alto.
    CHECK((NoArco(de, para, 150.0f, 0.0f) - de).Length() < 1e-3f);
    CHECK((NoArco(de, para, 150.0f, 1.0f) - para).Length() < 1e-3f);
    float maisAlto = 1e9f;
    for (int k = 0; k <= 20; ++k) maisAlto = std::min(maisAlto, NoArco(de, para, 150.0f, static_cast<float>(k) / 20.0f).y);
    CHECK(maisAlto < para.y);

    const int n = Fichas(30);
    // Antes de sair, parada; voando, no meio do caminho; depois, no losango.
    CHECK_FALSE(FichaEm(n - 1, n, 0.0f, de, para).voando);
    const Ficha meio = FichaEm(0, n, 0.5f * kVooDaFicha, de, para);
    CHECK(meio.voando);
    CHECK(meio.onde.y < de.y);
    CHECK((FichaEm(0, n, kVooDaFicha + 0.01f, de, para).onde - para).Length() < 1e-3f);
    // A ultima chega no carimbo.
    CHECK(FichaEm(n - 1, n, Carimbo(n) - 0.01f, de, para).voando);
    CHECK_FALSE(FichaEm(n - 1, n, Carimbo(n) + 0.01f, de, para).voando);
    // Elas saem de lugares diferentes da fileira.
    CHECK(FichaEm(0, n, 0.0f, de, para).onde.x < FichaEm(n - 1, n, 0.0f, de, para).onde.x);
    // A moeda gira: em algum momento do voo, quase de lado.
    float maisFina = 1.0f;
    for (int k = 1; k < 40; ++k) maisFina = std::min(maisFina, FichaEm(0, n, kVooDaFicha * static_cast<float>(k) / 40.0f, de, para).largura);
    CHECK(maisFina < 0.2f);
}

TEST_CASE("Festa da matricula: o losango da dois pulinhos, o anel abre e as faiscas caem") {
    CHECK(Pulo(-0.1f) == doctest::Approx(0.0f));
    CHECK(Pulo(0.3f * kDuracaoDoPulo) == doctest::Approx(kPrimeiroPulo));
    const float segundo = Pulo(0.8f * kDuracaoDoPulo);
    CHECK(segundo > 0.0f);
    CHECK(segundo < kPrimeiroPulo);
    CHECK(Pulo(kDuracaoDoPulo) == doctest::Approx(0.0f));
    for (int k = 0; k <= 50; ++k) CHECK(Pulo(kDuracaoDoPulo * static_cast<float>(k) / 50.0f) >= 0.0f);

    const Anel cedo = AnelEm(0.05f, 64.0f), tarde = AnelEm(0.4f, 64.0f);
    CHECK(cedo.raio >= 64.0f);
    CHECK(tarde.raio > cedo.raio);
    CHECK(tarde.opacidade < cedo.opacidade);
    CHECK(AnelEm(kVidaDoAnel, 64.0f).opacidade == doctest::Approx(0.0f));

    const Vector2 c(400.0f, 300.0f);
    const auto logo = Faiscas(c, 0.15f, kFaiscas);
    REQUIRE(logo.size() == static_cast<size_t>(kFaiscas));
    for (const Faisca& f : logo) CHECK(f.onde.y < c.y);   // espirram para cima
    // E a gravidade as traz de volta: no fim da vida, a media ja desce.
    float antes = 0.0f, depois = 0.0f;
    for (const Faisca& f : Faiscas(c, 0.3f, kFaiscas)) antes += f.onde.y;
    for (const Faisca& f : Faiscas(c, 0.85f, kFaiscas)) depois += f.onde.y;
    CHECK(depois > antes);
    CHECK(Faiscas(c, kVidaDaFaisca, kFaiscas).empty());

    CHECK(Duracao(4) > Carimbo(4));
    CHECK(Duracao(8) > Duracao(4));
    CHECK(Duracao(8) < 3.0f);   // rapida: a tela nao fica presa nela
}
