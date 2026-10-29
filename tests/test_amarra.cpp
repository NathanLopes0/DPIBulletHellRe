//
// A letra amarrada: a corda que desenrola, gira com o no e e cortada.
//

#include "doctest.h"

#include <cmath>
#include <vector>

#include "../Source/Attacks/Shared/Amarra.h"

namespace {

    constexpr float DT = 1.0f / 60.0f;
    constexpr float PI = 3.14159265f;

    struct Letra {
        Vector2 pos;
        Vector2 vel;
    };

    /// Um quadro NA ORDEM DO MOTOR: o RigidBody integra, e so depois a Motion.
    Amarra::Evento Quadro(Amarra::Corda& corda, Letra& l, const Vector2 no) {
        l.pos += l.vel * DT;
        const auto lance = Amarra::Passo(corda, l.pos, l.vel, no, DT);
        l.pos = lance.posicao;
        l.vel = lance.velocidade;
        return lance.evento;
    }

    Amarra::Corda UmaCorda(const float comprimento, const float duracao,
                           const Amarra::Sentido sentido = Amarra::Sentido::Horario) {
        Amarra::Corda c;
        c.comprimento = comprimento;
        c.duracao = duracao;
        c.sentido = sentido;
        return c;
    }
}

TEST_CASE("Amarra: a corda desenrola ate esticar, e dali a letra nao se afasta") {
    auto corda = UmaCorda(142.0f, 10.0f);
    const Vector2 no(400.0f, 150.0f);
    Letra l{no, Vector2(0.0f, 200.0f)};   // sai do no, para baixo

    // 142 px a 200 px/s sao 42,6 quadros: 42 quadros frouxa, e no 43o a corda
    // estica. (Um comprimento que caisse num quadro exato, como 140, deixaria
    // o teste a merce do arredondamento da soma.)
    int frouxa = 0;
    while (Quadro(corda, l, no) != Amarra::Evento::Amarrou) {
        ++frouxa;
        REQUIRE(frouxa < 100);
        CHECK(corda.amarrada < 0.0f);
        CHECK(l.pos.x == doctest::Approx(400.0f));   // reta, para baixo
    }
    CHECK(frouxa == 42);

    for (int i = 0; i < 300; ++i) {
        Quadro(corda, l, no);
        CHECK((l.pos - no).Length() == doctest::Approx(142.0f).epsilon(0.001));
    }
}

TEST_CASE("Amarra: o modulo da velocidade e o mesmo - o contrato de toda Motion") {
    auto corda = UmaCorda(120.0f, 10.0f);
    const Vector2 no(300.0f, 200.0f);
    Letra l{no, Vector2(150.0f, 0.0f)};
    for (int i = 0; i < 400; ++i) {
        Quadro(corda, l, no);
        CHECK(l.vel.Length() == doctest::Approx(150.0f).epsilon(0.001));
    }
}

TEST_CASE("Amarra: no sentido horario da tela, a letra a direita do no desce") {
    const Vector2 no(0.0f, 0.0f);

    auto horario = UmaCorda(100.0f, 10.0f, Amarra::Sentido::Horario);
    Letra h{no, Vector2(200.0f, 0.0f)};          // sai para a direita
    while (Quadro(horario, h, no) != Amarra::Evento::Amarrou) {}
    Quadro(horario, h, no);
    CHECK(h.vel.y > 0.0f);                       // y da tela para baixo
    CHECK(h.pos.y > 0.0f);

    auto anti = UmaCorda(100.0f, 10.0f, Amarra::Sentido::AntiHorario);
    Letra a{no, Vector2(200.0f, 0.0f)};
    while (Quadro(anti, a, no) != Amarra::Evento::Amarrou) {}
    Quadro(anti, a, no);
    CHECK(a.vel.y < 0.0f);
    CHECK(a.pos.y < 0.0f);
}

TEST_CASE("Amarra: a letra gira o angulo da velocidade sobre o comprimento") {

    // 200 px/s numa corda de 140 px sao 1,43 rad/s: em 2 s amarrada ela anda
    // 2,86 rad, uns 164 graus.
    auto corda = UmaCorda(140.0f, 10.0f);
    const Vector2 no(0.0f, 0.0f);
    Letra l{no, Vector2(200.0f, 0.0f)};
    while (Quadro(corda, l, no) != Amarra::Evento::Amarrou) {}

    float angulo = 0.0f;
    Vector2 antes = l.pos;
    for (int i = 0; i < 120; ++i) {
        Quadro(corda, l, no);
        angulo += std::atan2(antes.x * l.pos.y - antes.y * l.pos.x,
                             antes.x * l.pos.x + antes.y * l.pos.y);
        antes = l.pos;
    }
    CHECK(angulo == doctest::Approx(2.0f * 200.0f / 140.0f).epsilon(0.001));
}

TEST_CASE("Amarra: com o no andando, a roda continua sendo uma roda") {

    // O CASO QUE DERRUBOU A PRIMEIRA VERSAO: o professor varre a 200 px/s e a
    // roda sai a 200 px/s. Girando no espaco da tela, as letras se amontoavam
    // num arco atras dele, e a que saia para a frente nunca esticava a corda.
    // E a corda nasce no no, e nao na distancia do primeiro quadro: com ela, a
    // de tras saia com dois quadros de vantagem, e a roda ficava torta em
    // 0,024 rad.
    const int n = 8;
    Vector2 no(200.0f, 150.0f);
    std::vector<Amarra::Corda> cordas(n, UmaCorda(140.0f, 10.0f));
    std::vector<Letra> letras;
    for (int i = 0; i < n; ++i) {
        const float a = 2.0f * PI * static_cast<float>(i) / n;
        letras.push_back({no, Vector2(std::cos(a), std::sin(a)) * 200.0f});
    }

    for (int q = 0; q < 120; ++q) {                  // 2 s de varredura
        no += Vector2(200.0f * DT, 0.0f);
        for (int i = 0; i < n; ++i) Quadro(cordas[i], letras[i], no);
    }

    for (int i = 0; i < n; ++i) {
        CAPTURE(i);
        CHECK(cordas[i].amarrada >= 0.0f);           // inclusive a da frente (i = 0)
        CHECK((letras[i].pos - no).Length() == doctest::Approx(140.0f).epsilon(0.001));
    }
    // E as oito continuam a um oitavo de volta uma da outra.
    for (int i = 0; i < n; ++i) {
        const Vector2 a = letras[i].pos - no, b = letras[(i + 1) % n].pos - no;
        const float entre = std::atan2(a.x * b.y - a.y * b.x, a.x * b.x + a.y * b.y);
        CHECK(entre == doctest::Approx(2.0f * PI / n).epsilon(0.001));
    }
}

TEST_CASE("Amarra: passada a duracao, a corda e cortada e a letra segue reta pela tangente") {
    auto corda = UmaCorda(100.0f, 1.0f);
    const Vector2 no(0.0f, 0.0f);
    Letra l{no, Vector2(0.0f, 180.0f)};

    int amarrou = -1, cortou = -1;
    for (int i = 0; i < 200 && cortou < 0; ++i) {
        const auto e = Quadro(corda, l, no);
        if (e == Amarra::Evento::Amarrou) amarrou = i;
        if (e == Amarra::Evento::Cortou)  cortou = i;
    }
    REQUIRE(amarrou >= 0);
    REQUIRE(cortou >= 0);
    CHECK(corda.cortada);
    // Cortada no primeiro quadro em que o tempo amarrada chegou a 1 s. Em
    // quadros sao 60 ou 61, conforme o arredondamento da soma de 1/60.
    CHECK(corda.amarrada >= 1.0f);
    CHECK(corda.amarrada < 1.0f + DT);
    CHECK(cortou - amarrou >= 60);
    CHECK(cortou - amarrou <= 61);

    // A velocidade do corte e a tangente: perpendicular a corda.
    CHECK(std::fabs(l.vel.x * l.pos.x + l.vel.y * l.pos.y) < 0.01f * 180.0f * 100.0f);

    // Cortada, a velocidade nao muda mais, e a letra se afasta do no.
    const Vector2 velocidade = l.vel;
    for (int i = 0; i < 60; ++i) {
        Quadro(corda, l, no);
        CHECK(l.vel.x == doctest::Approx(velocidade.x));
        CHECK(l.vel.y == doctest::Approx(velocidade.y));
    }
    CHECK((l.pos - no).Length() > 200.0f);
}

TEST_CASE("Amarra: uma letra parada nao e amarrada, por mais longe que esteja") {
    // E a letra adormecida de uma WaveAttack: presa a corda sem velocidade, ela
    // seria arrastada pelo professor sem nunca girar.
    auto corda = UmaCorda(50.0f, 1.0f);
    Letra l{Vector2(500.0f, 0.0f), Vector2::Zero};
    const auto e = Quadro(corda, l, Vector2(0.0f, 0.0f));
    CHECK(e == Amarra::Evento::Nenhum);
    CHECK(corda.solta < 0.0f);
    CHECK(corda.amarrada < 0.0f);
    CHECK(l.pos.x == doctest::Approx(500.0f));
}

TEST_CASE("Amarra: os nomes do sentido") {
    Amarra::Sentido s = Amarra::Sentido::AntiHorario;
    CHECK(Amarra::SentidoPeloNome("horario", s));
    CHECK(s == Amarra::Sentido::Horario);
    CHECK(Amarra::SentidoPeloNome("antihorario", s));
    CHECK(s == Amarra::Sentido::AntiHorario);
    CHECK_FALSE(Amarra::SentidoPeloNome("Horario", s));
    CHECK_FALSE(Amarra::SentidoPeloNome("anti-horario", s));
    CHECK_FALSE(Amarra::SentidoPeloNome("", s));
}
