// Testes da investida repetida: o Modifier que pulsa o modulo e a Motion que
// re-aponta em instantes discretos. So a camada pura, sem SDL.

#include "doctest.h"
#include "../Source/Attacks/PathAim.h"

namespace {
    constexpr float DT = 1.0f / 60.0f;
    float anguloEntre(const Vector2& a, const Vector2& b) {
        const float g = Math::ToDegrees(Math::Atan2(a.y, a.x) - Math::Atan2(b.y, b.x));
        float d = Math::Abs(g);
        if (d > 180.0f) d = 360.0f - d;
        return d;
    }
}

// ---------------------------------------------------------------------------
// AjustarModulo — o espelho de DirecionarPreservandoModulo
// ---------------------------------------------------------------------------

TEST_CASE("AjustarModulo: define o modulo pedido") {
    const Vector2 v = AjustarModulo(Vector2(30.f, 40.f), 200.f);
    CHECK(v.Length() == doctest::Approx(200.f));
}

TEST_CASE("AjustarModulo: preserva a direcao") {
    const Vector2 original(30.f, 40.f);
    const Vector2 v = AjustarModulo(original, 7.f);
    CHECK(anguloEntre(v, original) == doctest::Approx(0.f).epsilon(0.001));
}

TEST_CASE("AjustarModulo: velocidade nula fica como esta") {
    // Nao ha direcao a preservar; inventar uma seria decidir pelo chamador.
    const Vector2 v = AjustarModulo(Vector2::Zero, 500.f);
    CHECK(v.Length() == doctest::Approx(0.f));
}

TEST_CASE("AjustarModulo: modulo negativo vira zero") {
    const Vector2 v = AjustarModulo(Vector2(10.f, 0.f), -50.f);
    CHECK(v.Length() == doctest::Approx(0.f));
}

TEST_CASE("AjustarModulo e DirecionarPreservandoModulo sao complementares") {
    // Um troca o modulo e mantem a direcao; o outro troca a direcao e mantem o
    // modulo. Aplicados em sequencia, o resultado tem o modulo de um e a
    // direcao do outro.
    const Vector2 v = DirecionarPreservandoModulo(AjustarModulo(Vector2(3.f, 4.f), 100.f),
                                                  Vector2(0.f, 1.f), 1.f);
    CHECK(v.Length() == doctest::Approx(100.f));
    CHECK(anguloEntre(v, Vector2(0.f, 1.f)) == doctest::Approx(0.f).epsilon(0.001));
}

// ---------------------------------------------------------------------------
// CalcularFaseDoPulso — toda a aritmetica de tempo do Modifier
// ---------------------------------------------------------------------------

namespace {
    FaseDoPulso fase(float t) { return CalcularFaseDoPulso(t, 0.5f, 0.2f, 1.0f, 800.f, 15.f, 3); }
}

TEST_CASE("Pulso: antes do atraso nao esta ativo nem terminou") {
    const auto f = fase(0.3f);
    CHECK_FALSE(f.ativo);
    CHECK(f.ciclo == 0);
}

TEST_CASE("Pulso: a investida vem primeiro, e dura o que foi pedido") {
    CHECK(fase(0.55f).investindo);
    CHECK(fase(0.55f).modulo == doctest::Approx(800.f));
    CHECK(fase(0.69f).investindo);
    CHECK_FALSE(fase(0.75f).investindo);
    CHECK(fase(0.75f).modulo == doctest::Approx(15.f));
}

TEST_CASE("Pulso: conta os ciclos") {
    CHECK(fase(0.6f).ciclo == 0);
    CHECK(fase(1.9f).ciclo == 1);   // 0.5 + 1.2
    CHECK(fase(2.95f).ciclo == 2);  // ja no terceiro ciclo, ainda investindo
}

TEST_CASE("Pulso: a sequencia termina numa INVESTIDA, nunca numa pausa") {
    // Sao N investidas com N-1 pausas entre elas. Terminar na pausa deixaria o
    // projetil lento demais para sair da tela, e ele viraria um obstaculo
    // permanente ocupando uma vaga do pool.
    const auto ultimaInvestida = fase(2.95f);          // 0.5 + 2*1.2 + 0.05
    CHECK(ultimaInvestida.ativo);
    CHECK(ultimaInvestida.investindo);
    CHECK(ultimaInvestida.modulo == doctest::Approx(800.f));

    // A ultima investida vai de t=2.9 a t=3.1; logo depois viria a pausa.
    const auto logoDepois = fase(3.15f);               // a pausa que nao acontece
    CHECK_FALSE(logoDepois.ativo);
    CHECK(logoDepois.ciclo == 3);                      // encerrou

    // Como a ultima fase ativa foi uma investida, a velocidade que fica valendo
    // e a alta - e o projetil sai da tela sozinho.
    CHECK(ultimaInvestida.modulo > 100.f);
}

TEST_CASE("Pulso: depois da ultima repeticao para de agir") {
    const auto f = fase(10.0f);
    CHECK_FALSE(f.ativo);
    CHECK(f.ciclo == 3);            // e por isso que o Modifier se encerra
}

TEST_CASE("Pulso: zero repeticoes encerra de imediato") {
    const auto f = CalcularFaseDoPulso(1.f, 0.f, 0.2f, 1.f, 800.f, 15.f, 0);
    CHECK_FALSE(f.ativo);
    CHECK(f.ciclo <= 0);
}

// ---------------------------------------------------------------------------
// A investida inteira, simulada na ordem do motor
// ---------------------------------------------------------------------------

TEST_CASE("Investida repetida: re-aponta e pulsa, sem uma peca saber da outra") {
    const Vector2 jogador(600.f, 600.f);
    Vector2 pos(600.f, 120.f), vel(0.f, 200.f);

    const float intervalo = 1.2f, dInvestida = 0.2f, dPausa = 1.0f;
    const float vInvestida = 700.f, vPausa = 15.f;
    const int repeticoes = 6;

    float t = 0.f, proximaMirada = 0.f;
    int miradas = 0;
    float piorErro = 0.f, menorModuloNaInvestida = 1e9f, maiorModuloNaPausa = 0.f;
    float menorDistancia = 1e9f;

    while (t < intervalo * repeticoes) {
        pos += vel * DT;                                   // 1. RigidBody

        if (miradas < repeticoes && t >= proximaMirada) {  // 2. Motion
            const Vector2 paraJogador = jogador - pos;
            vel = DirecionarPreservandoModulo(vel, paraJogador, vel.Length());
            piorErro = Math::Max(piorErro, anguloEntre(vel, paraJogador));
            ++miradas;
            proximaMirada += intervalo;
        }

        const auto f = CalcularFaseDoPulso(t, 0.f, dInvestida, dPausa,   // 3. Modifier
                                           vInvestida, vPausa, repeticoes);
        if (f.ativo) {
            vel = AjustarModulo(vel, f.modulo);
            if (f.investindo) menorModuloNaInvestida = Math::Min(menorModuloNaInvestida, vel.Length());
            else              maiorModuloNaPausa = Math::Max(maiorModuloNaPausa, vel.Length());
        }
        menorDistancia = Math::Min(menorDistancia, (pos - jogador).Length());
        t += DT;
    }

    CHECK(miradas == repeticoes);
    CHECK(piorErro < 0.5f);                                    // sempre aponta para o jogador
    CHECK(menorModuloNaInvestida == doctest::Approx(vInvestida));
    CHECK(maiorModuloNaPausa == doctest::Approx(vPausa));
    CHECK(menorDistancia < 200.f);                             // rondou o jogador durante a cacada

    // E o mais importante para o pool: termina em velocidade de investida, nao
    // de pausa. Terminar devagar deixaria o projetil preso na tela.
    CHECK(vel.Length() == doctest::Approx(vInvestida));
    CHECK((pos - jogador).Length() > 300.f);                   // ja esta indo embora
}

TEST_CASE("Investida repetida: a pausa nao pode ser parada de verdade") {
    // A direcao mora dentro do vetor velocidade. Com modulo zero ela se perde, e
    // a mirada seguinte nao tem o que girar.
    Vector2 parado = AjustarModulo(Vector2(0.f, 200.f), 0.f);
    const Vector2 reapontado = DirecionarPreservandoModulo(parado, Vector2(100.f, 0.f), parado.Length());
    CHECK(reapontado.Length() == doctest::Approx(0.f));

    // Com um resto de velocidade, a mirada funciona.
    Vector2 quaseParado = AjustarModulo(Vector2(0.f, 200.f), 15.f);
    const Vector2 ok = DirecionarPreservandoModulo(quaseParado, Vector2(100.f, 0.f), quaseParado.Length());
    CHECK(ok.Length() == doctest::Approx(15.f));
    CHECK(ok.x == doctest::Approx(15.f));
}
