//
// As fases 1 e 2 do Lucas: a <div> e as regras CSS e JavaScript.
//

#include "doctest.h"

#include <cmath>
#include <set>
#include <string>

#include "../Source/Attacks/Dom.h"

using namespace Dom;

namespace {
    const Campo kCampo{0.0f, 0.0f, 1200.0f, 720.0f};
}

TEST_CASE("Dom: a div nasce pequena, quica nas bordas e nunca sai do campo") {
    Variacao::Sorteio s(7u);
    Div d = Nova(kCampo, 110.0f, s);
    CHECK(d.largura == doctest::Approx(kLarguraInicial));
    CHECK(d.velocidade.Length() == doctest::Approx(110.0f));
    for (int k = 0; k < 4000; ++k) {
        Andar(d, 0.016f, k * 0.016f, Vector2(600.0f, 650.0f), kCampo, s);
        CHECK(d.centro.x - d.largura / 2.0f >= kCampo.x - 0.01f);
        CHECK(d.centro.x + d.largura / 2.0f <= kCampo.x + kCampo.w + 0.01f);
        CHECK(d.centro.y - d.altura / 2.0f >= kCampo.y - 0.01f);
        CHECK(d.centro.y + d.altura / 2.0f <= kCampo.y + kCampo.h + 0.01f);
    }
}

TEST_CASE("Dom: as regras CSS mudam a forma e se somam; as que aliviam aliviam") {
    Variacao::Sorteio s(1u);
    Div d = Nova(kCampo, 110.0f, s);
    const float l0 = d.largura;
    CHECK_FALSE(Aplicar(d, Efeito::Largura, 1.0f));
    CHECK(d.largura > l0);
    const float l1 = d.largura;
    Aplicar(d, Efeito::Largura, 1.0f);
    CHECK(d.largura > l1);                                   // acumula
    for (int k = 0; k < 10; ++k) Aplicar(d, Efeito::Largura, 1.0f);
    CHECK(d.largura == doctest::Approx(kMaiorLargura));      // com teto
    CHECK(d.mudancas == 12);
    // display: none some com ela por um tempo.
    Aplicar(d, Efeito::Some, 2.0f);
    CHECK_FALSE(Visivel(d, 2.0f + kSome * 0.5f));
    CHECK(Caixas(d, 2.0f + kSome * 0.5f).empty());
    CHECK(Visivel(d, 2.0f + kSome + 0.01f));
    // display: grid vira quatro caixas.
    Aplicar(d, Efeito::Grade, 10.0f);
    CHECK(Caixas(d, 10.0f).size() == 4u);
    // width: 50% encolhe, com piso.
    for (int k = 0; k < 20; ++k) Aplicar(d, Efeito::Estreita, 10.0f);
    CHECK(d.largura == doctest::Approx(kMenorLado));
    // position: fixed para a div.
    Aplicar(d, Efeito::Fixa, 20.0f);
    const Vector2 antes = d.centro;
    Andar(d, 0.5f, 21.0f, Vector2(0.0f, 0.0f), kCampo, s);
    CHECK((d.centro - antes).Length() < 0.01f);
    CHECK(Ajuda(Efeito::Some));
    CHECK(Ajuda(Efeito::Remove));
    CHECK_FALSE(Ajuda(Efeito::Largura));
    CHECK_FALSE(Ajuda(Efeito::Intervalo));
}

TEST_CASE("Dom: as regras JavaScript dao comportamento - atirar, revidar, explodir, clonar") {
    Variacao::Sorteio s(3u);
    Div d = Nova(kCampo, 110.0f, s);
    CHECK(Atirar(d, 5.0f) == Disparo::Nenhum);
    Aplicar(d, Efeito::Intervalo, 0.0f);
    CHECK(Atirar(d, 0.5f) == Disparo::Nenhum);
    CHECK(Atirar(d, 1.0f) == Disparo::Anel);
    CHECK(Atirar(d, 1.1f) == Disparo::Nenhum);
    const float antes = d.intervalo;
    Aplicar(d, Efeito::Intervalo, 2.0f);
    CHECK(d.intervalo < antes);                               // mais um setInterval aperta o ritmo
    Aplicar(d, Efeito::Bomba, 10.0f);
    CHECK(Atirar(d, 10.0f + kPavio) == Disparo::Explosao);
    CHECK(d.bombaEm < 0.0f);                                  // explode uma vez
    CHECK_FALSE(Revidar(d, 1.0f));
    Aplicar(d, Efeito::Revide, 1.0f);
    CHECK(Revidar(d, 1.0f));
    CHECK_FALSE(Revidar(d, 1.0f + kEntreRevides * 0.5f));
    CHECK(Revidar(d, 1.0f + kEntreRevides));
    CHECK(Aplicar(d, Efeito::Clone, 3.0f));                   // a batalha clona
    // mousemove: a div vira para o aluno.
    Div seguidora = Nova(kCampo, 110.0f, s);
    Aplicar(seguidora, Efeito::Segue, 0.0f);
    const Vector2 aluno(600.0f, 680.0f);
    const float longe = (seguidora.centro - aluno).Length();
    for (int k = 0; k < 120; ++k) Andar(seguidora, 0.016f, k * 0.016f, aluno, kCampo, s);
    CHECK((seguidora.centro - aluno).Length() < longe);
    // Todas as regras tem texto, e cada linguagem sorteia so as dela.
    for (const Efeito e : kDoCss) { CHECK_FALSE(EhJs(e)); CHECK(std::string(Texto(e)).find(';') != std::string::npos); }
    for (const Efeito e : kDoJs) { CHECK(EhJs(e)); CHECK_FALSE(std::string(Texto(e)).empty()); }
    std::set<int> vistas;
    for (int k = 0; k < 400; ++k) {
        const Efeito e = Sortear(true, s);
        CHECK(EhJs(e));
        vistas.insert(static_cast<int>(e));
    }
    CHECK(vistas.size() == std::size(kDoJs));
}

TEST_CASE("Dom: as pecas cobrem a div, e a redonda so o circulo") {
    Variacao::Sorteio s(5u);
    Div d = Nova(kCampo, 0.0f, s);
    d.centro = Vector2(600.0f, 300.0f);
    d.largura = 120.0f;
    d.altura = 60.0f;
    const auto pecas = Pecas(d, 0.0f, 30.0f);
    CHECK(pecas.size() == 8u);
    for (const Vector2& p : pecas) CHECK(Toca(d, p, 0.0f, 0.0f));
    CHECK_FALSE(Toca(d, Vector2(600.0f, 400.0f), 10.0f, 0.0f));
    CHECK(Toca(d, Vector2(600.0f, 335.0f), 10.0f, 0.0f));
    d.redonda = true;
    for (const Vector2& p : Pecas(d, 0.0f, 20.0f)) CHECK((p - d.centro).Length() <= 30.0f + 0.01f);
    // O Dificil e mais apertado em tudo.
    const Degrau n = DegrauDo(Dificuldade::Nivel::Normal), df = DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(df.entreLances < n.entreLances);
    CHECK(df.regrasPorLance > n.regrasPorLance);
    CHECK(df.velocidadeDasRegras > n.velocidadeDasRegras);
}

TEST_CASE("Dom: a regra mirada na div faz a curva, mas nao vira de uma vez") {
    // Indo para a direita, com a div bem acima: num quadro, vira so o giro de um quadro.
    const Vector2 v(170.0f, 0.0f);
    const Vector2 um = Curvar(v, Vector2(0.0f, 0.0f), Vector2(0.0f, -300.0f), kGiroDaRegra, 0.016f);
    CHECK(um.Length() == doctest::Approx(170.0f));
    const float virou = -Math::ToDegrees(std::atan2(um.y, um.x));
    CHECK(virou == doctest::Approx(kGiroDaRegra * 0.016f).epsilon(0.01));
    // Com tempo, aponta para a div, e nao passa dela.
    const Vector2 muito = Curvar(v, Vector2(0.0f, 0.0f), Vector2(0.0f, -300.0f), kGiroDaRegra, 5.0f);
    CHECK(muito.x == doctest::Approx(0.0f).epsilon(0.001));
    CHECK(muito.y == doctest::Approx(-170.0f));
    // Ja na direcao certa, nada muda.
    const Vector2 reta = Curvar(v, Vector2(0.0f, 0.0f), Vector2(500.0f, 0.0f), kGiroDaRegra, 0.016f);
    CHECK(reta.x == doctest::Approx(170.0f));
    CHECK(reta.y == doctest::Approx(0.0f));
}
