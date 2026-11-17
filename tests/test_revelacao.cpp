// A revelacao da materia secreta.

#include "doctest.h"

#include "../Source/Curso/Revelacao.h"

#include <string>

using namespace Revelacao;

TEST_CASE("Revelacao: os tempos vem na ordem, e o botao so vale depois dos parabens") {
    CHECK(kEscuro < kComecaACarga);
    CHECK(kComecaACarga < kEstouro);
    CHECK(kEstouro < kTitulo);
    CHECK(kTitulo < kNome);
    CHECK(kNome < kPodeSeguir);
    CHECK_FALSE(Aviso(kPodeSeguir - 0.01f));
    CHECK(Aviso(kPodeSeguir + 0.1f));
}

TEST_CASE("Revelacao: o veu cobre a tela, fica, e sai na ida") {
    CHECK(Veu(0.0f, -1.0f) == doctest::Approx(0.0f));
    CHECK(Veu(kEscuro, -1.0f) == doctest::Approx(kVeuCheio));
    CHECK(Veu(10.0f, -1.0f) == doctest::Approx(kVeuCheio));   // o aluno ainda nao apertou
    CHECK(Veu(10.0f, kIda / 2.0f) < kVeuCheio);
    CHECK(Veu(10.0f, kIda) == doctest::Approx(0.0f));
}

TEST_CASE("Revelacao: a carga so existe entre o escuro e o estouro, e converge para o meio") {
    const Vector2 meio(600.0f, 360.0f);
    CHECK(Carga(meio, kComecaACarga - 0.01f, kPixelsDaCarga).empty());
    CHECK(Carga(meio, kEstouro, kPixelsDaCarga).empty());
    const auto distanciaMedia = [&](const float t) {
        const auto pixels = Carga(meio, t, kPixelsDaCarga);
        REQUIRE_FALSE(pixels.empty());
        float soma = 0.0f;
        for (const Pixel& p : pixels) soma += (p.onde - meio).Length();
        return soma / static_cast<float>(pixels.size());
    };
    CHECK(distanciaMedia(kEstouro - 0.01f) < distanciaMedia(kComecaACarga + 0.6f));
    CHECK(distanciaMedia(kEstouro - 0.01f) < 60.0f);
    // Dourados e ciano, metade de cada.
    int dourados = 0;
    const auto todos = Carga(meio, kEstouro - 0.01f, kPixelsDaCarga);
    for (const Pixel& p : todos) dourados += p.dourado;
    CHECK(dourados * 2 == static_cast<int>(todos.size()));
}

TEST_CASE("Revelacao: a tela treme na carga, mais forte no estouro, e para") {
    CHECK(Tremor(0.0f) == doctest::Approx(0.0f));
    CHECK(Tremor(kEstouro - 0.1f) > 0.0f);
    CHECK(Tremor(kEstouro + 0.01f) > Tremor(kEstouro - 0.1f));
    CHECK(Tremor(kEstouro + 1.0f) == doctest::Approx(0.0f));
    CHECK(Sacudida(kEstouro + 1.0f).Length() == doctest::Approx(0.0f));
}

TEST_CASE("Revelacao: o estouro - o clarao, as tres ondas e as faiscas") {
    const Vector2 meio(600.0f, 360.0f);
    CHECK(Clarao(kEstouro - 0.01f) == doctest::Approx(0.0f));
    CHECK(Clarao(kEstouro) == doctest::Approx(1.0f));
    CHECK(Clarao(kEstouro + 1.0f) == doctest::Approx(0.0f));
    CHECK(Ondas(kEstouro - 0.01f).empty());
    CHECK(Ondas(kEstouro + 0.3f).size() == 3u);
    CHECK(Ondas(kEstouro + 5.0f).empty());
    const auto faiscas = Faiscas(meio, kEstouro + 0.2f, kFaiscas);
    CHECK(faiscas.size() == static_cast<size_t>(kFaiscas));
    CHECK(Faiscas(meio, kEstouro + kVidaDaFaisca + 0.01f, kFaiscas).empty());
    // O mesmo instante desenha sempre igual.
    CHECK(Faiscas(meio, kEstouro + 0.2f, kFaiscas)[5].onde.x == doctest::Approx(faiscas[5].onde.x));
}

TEST_CASE("Revelacao: o losango nasce no estouro, passa do tamanho e volta") {
    CHECK(Escala(kEstouro - 0.01f) == doctest::Approx(0.0f));
    CHECK(Escala(kEstouro + 0.18f) == doctest::Approx(kEscalaGrande * 1.3f));
    CHECK(Escala(kEstouro + 0.45f) == doctest::Approx(kEscalaGrande));
    CHECK(Escala(kEstouro + 3.0f) == doctest::Approx(kEscalaGrande).epsilon(0.04));
}

TEST_CASE("Revelacao: a ida leva o losango grande ate o lugar dele, no tamanho do botao") {
    const Vector2 meio(600.0f, 360.0f), lugar(900.0f, 500.0f);
    const Ida comeco = IdaEm(meio, lugar, 0.0f);
    CHECK(comeco.onde.x == doctest::Approx(meio.x));
    CHECK(comeco.escala == doctest::Approx(kEscalaGrande));
    CHECK_FALSE(comeco.chegou);
    const Ida meioDoCaminho = IdaEm(meio, lugar, kIda / 2.0f);
    CHECK(meioDoCaminho.onde.y < (meio.y + lugar.y) / 2.0f);   // em arco: sobe
    const Ida fim = IdaEm(meio, lugar, kIda);
    CHECK(fim.chegou);
    CHECK(fim.onde.x == doctest::Approx(lugar.x));
    CHECK(fim.onde.y == doctest::Approx(lugar.y));
    CHECK(fim.escala == doctest::Approx(1.0f));
}

TEST_CASE("Revelacao: os parabens contam as materias do curso") {
    const std::string frase = Parabens(17);
    CHECK(frase.find("17") != std::string::npos);
    CHECK(frase.find("Parab") == 0);
}
