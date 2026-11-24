//
// A final do Lucas: o Phoenix e a fenix.
//

#include "doctest.h"

#include "../Source/Attacks/Fenix.h"

using namespace Fenix;

namespace {
    const Campo kCampo{0.0f, 0.0f, 1200.0f, 720.0f};
}

TEST_CASE("Fenix: o voo em oito fica no alto do campo, e renascer a deixa mais rapida") {
    for (float t = 0.0f; t < 12.0f; t += 0.05f) {
        const Vector2 p = NoVoo(t, 600.0f, kCampo, 6.0f, 0, 1.15f);
        CHECK(p.x >= 40.0f - 0.01f);
        CHECK(p.x <= 1160.0f + 0.01f);
        CHECK(p.y < kCampo.h * 0.5f);
    }
    // Uma volta inteira a traz de volta ao comeco.
    CHECK((NoVoo(6.0f, 600.0f, kCampo, 6.0f, 0, 1.15f) - NoVoo(0.0f, 600.0f, kCampo, 6.0f, 0, 1.15f)).Length() < 0.5f);
    // A versao 1 da a volta antes.
    const Vector2 v0 = NoVoo(1.0f, 600.0f, kCampo, 6.0f, 0, 1.15f);
    const Vector2 v1 = NoVoo(1.0f, 600.0f, kCampo, 6.0f, 1, 1.15f);
    CHECK((v0 - v1).Length() > 1.0f);
}

TEST_CASE("Fenix: o LiveView segue o aluno com atraso, e os sockets ficam longe do pe do campo") {
    float x = 200.0f;
    x = SeguirX(x, 900.0f, 1.0f);
    CHECK(x == doctest::Approx(200.0f + kRapidezDoLiveView));
    for (int k = 0; k < 100; ++k) x = SeguirX(x, 900.0f, 1.0f);
    CHECK(x == doctest::Approx(900.0f));
    for (const Vector2& s : Sockets(kCampo)) {
        CHECK(s.y <= kCampo.h * 0.7f);
        CHECK(s.x > 0.0f);
        CHECK(s.x < kCampo.w);
    }
    const Degrau n = DegrauDo(Dificuldade::Nivel::Normal), d = DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(d.entrePenas < n.entrePenas);
    CHECK(d.entreBroadcasts < n.entreBroadcasts);
    CHECK(kApagada > kRenascer);
}
