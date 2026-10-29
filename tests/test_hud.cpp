//
// A HUD da batalha: a moldura do campo, o relogio da fase e os efeitos.
//

#include "doctest.h"

#include "../Source/UI/Hud.h"

using namespace Hud;

TEST_CASE("Hud: a moldura fica onde o desenho do aluno para, e acende quando ele encosta") {
    const Caixa campo{0.0f, 0.0f, 1200.0f, 746.0f};
    const Caixa area = AreaDoAluno(campo, 32.0f, 64.0f);
    // O CENTRO para a uma largura de sprite (Player::BorderLimitCheck); o
    // desenho, a meia.
    CHECK(area.esquerda == doctest::Approx(16.0f));
    CHECK(area.direita == doctest::Approx(1184.0f));
    CHECK(area.topo == doctest::Approx(32.0f));
    CHECK(area.baixo == doctest::Approx(714.0f));

    const Vector2 meio(16.0f, 32.0f);
    // NO MEIO DO CAMPO, nada acende.
    const Encosto longe = Encostado(area, Vector2(600.0f, 400.0f), meio);
    CHECK(longe.esquerda == 0.0f);
    CHECK(longe.direita == 0.0f);
    CHECK(longe.topo == 0.0f);
    CHECK(longe.baixo == 0.0f);
    // NO LIMITE de baixo a esquerda (o centro onde o BorderLimitCheck o prende),
    // as duas paredes estao coladas.
    const Encosto canto = Encostado(area, Vector2(32.0f, 746.0f - 64.0f), meio);
    CHECK(canto.esquerda == doctest::Approx(1.0f));
    CHECK(canto.baixo == doctest::Approx(1.0f));
    CHECK(canto.direita == 0.0f);
    // A MEIO CAMINHO do aviso, meio aceso.
    const Encosto meioCaminho = Encostado(area, Vector2(1184.0f - 16.0f - kAvisoDaParede / 2.0f, 400.0f), meio);
    CHECK(meioCaminho.direita == doctest::Approx(0.5f));
}

TEST_CASE("Hud: o clarao some com a idade") {
    CHECK(Clarao(0.0f, 0.5f) == doctest::Approx(1.0f));
    CHECK(Clarao(0.25f, 0.5f) == doctest::Approx(0.25f));
    CHECK(Clarao(0.5f, 0.5f) == 0.0f);
    CHECK(Clarao(-1.0f, 0.5f) == 0.0f);   // nunca aceso
}

TEST_CASE("Hud: o relogio vai do verde ao vermelho, pulsa no fim e tem um dente a cada 5 s") {
    const Rgb cheio = CorDoRelogio(1.0f), meio = CorDoRelogio(0.5f), fim = CorDoRelogio(0.1f);
    CHECK(cheio.g > cheio.r);          // verde
    CHECK(meio.r == 255);              // ambar
    CHECK(fim.r == 255);
    CHECK(fim.g < meio.g);             // vermelho
    CHECK(Urgente(2.0f));
    CHECK_FALSE(Urgente(5.0f));
    CHECK_FALSE(Urgente(0.0f));
    const auto vinte = Marcas(20.0f);
    REQUIRE(vinte.size() == 3u);
    CHECK(vinte[0] == doctest::Approx(0.25f));
    CHECK(vinte[2] == doctest::Approx(0.75f));
    CHECK(Marcas(30.0f).size() == 5u);
    CHECK(Marcas(0.0f).empty());
}

TEST_CASE("Hud: as faiscas crescem, apagam e cada uma tem o seu tamanho") {
    for (const Faisca f : {Faisca::Acerto, Faisca::Derrubada, Faisca::Dano, Faisca::Coleta, Faisca::Dissolvido,
                          Faisca::Resposta}) {
        CHECK(Brilho(f, 0.0f) == doctest::Approx(1.0f));
        CHECK(Brilho(f, Vida(f)) == 0.0f);
        CHECK(Raio(f, Vida(f)) > Raio(f, 0.0f));
        CHECK(Raio(f, Vida(f) * 0.5f) <= Raio(f, Vida(f)));
    }
    // O DANO SE VE MAIS que o acerto: e o que o aluno nao pode deixar de notar.
    CHECK(Raio(Faisca::Dano, Vida(Faisca::Dano)) > Raio(Faisca::Acerto, Vida(Faisca::Acerto)));
    CHECK(Vida(Faisca::Acerto) < Vida(Faisca::Dano));
}

TEST_CASE("Hud: cada aluno tem a sua cor, e o balao barrado murcha ate um terco") {
    const Rgb um = CorDoAluno(0), dois = CorDoAluno(1), fora = CorDoAluno(7);
    CHECK(um.b > um.r);      // azul
    CHECK(dois.r > dois.b);  // laranja
    CHECK(fora.r == 255);
    CHECK(fora.g == 255);
    CHECK(Murcho(0.0f) == doctest::Approx(1.0f));
    CHECK(Murcho(kMurcha) == doctest::Approx(0.3f));
    CHECK(Murcho(kMurcha * 0.5f) < 1.0f);
    CHECK(Murcho(kMurcha * 0.5f) > 0.3f);
    CHECK(Murcho(10.0f) == doctest::Approx(0.3f));
}
